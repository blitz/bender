/*
 * \brief   Elf extraction.
 * \date    2006-06-07
 * \author  Bernhard Kauer <kauer@tudos.org>
 */
/*
 * Copyright (C) 2006  Bernhard Kauer <kauer@tudos.org>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the OSLO package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */

#include <acpi.h>
#include <elf.h>
#include <mbi-tools.h>
#include <mbi2.h>
#include <util.h>

enum { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI };

static void byte_out(uint8_t **code, uint8_t byte) {
  **code = byte;
  (*code)++;
  /* XXX Check overflow. */
}

static void gen_mov(uint8_t **code, int reg, uint32_t constant) {
  byte_out(code, 0xB8 | reg);
  *((uint32_t *)*code) = constant;
  *code += sizeof(uint32_t);
}

static void gen_jmp_edx(uint8_t **code) {
  byte_out(code, 0xFF);
  byte_out(code, 0xE2);
}

static void gen_elf_segment(uint8_t **code, uintptr_t target, void *src,
                            size_t len, size_t fill) {
  gen_mov(code, EDI, (uint32_t)target);
  gen_mov(code, ESI, (uint32_t)src);
  gen_mov(code, ECX, len);
  byte_out(code, 0xF3); /* REP */
  byte_out(code, 0xA4); /* MOVSB */
  /* EAX is zero at this point. */
  gen_mov(code, ECX, fill);
  byte_out(code, 0xF3); /* REP */
  byte_out(code, 0xAA); /* STOSB */
}

static struct mbi2_boot_info *
convert_mb1_to_mbi2(struct mbi *mbi, uint64_t phys_addr, size_t length) {
  struct mbi2_builder bld = mbi2_build(phys_addr, length);

  if (mbi->flags & MBI_FLAG_CMDLINE) {
    mbi2_add_boot_cmdline(&bld, (const char *)(uintptr_t)mbi->cmdline);
  }

  if (mbi->flags & MBI_FLAG_MEM) {
    mbi2_add_mbi1_memmap(&bld, (struct memory_map *)mbi->mmap_addr,
                         mbi->mmap_addr + mbi->mmap_length);
  }

  if (mbi->flags & MBI_FLAG_MODS) {
    for (uint32_t mod_index = 0; mod_index < mbi->mods_count; mod_index++) {
      struct module *mod =
          ((struct module *)(uintptr_t)mbi->mods_addr) + mod_index;

      printf("Adding module %u: %s\n", mod_index,
             (const char *)(uintptr_t)mod->string);
      mbi2_add_module(&bld, mod->mod_start, mod->mod_end,
                      (const char *)(uintptr_t)mod->string);
    }
  }

  /* Even some non-UEFI payloads require the RSDP to be provided by the
   * Multiboot2 loader. */
  const struct rsdp *rsdp = acpi_get_rsdp();
  if (rsdp) {
    printf("Adding RSDP info: %p (%d) -> %lx\n", rsdp, rsdp->rev, rsdp->rsdt);
    mbi2_add_rsdp(&bld, rsdp);
  }

  return (struct mbi2_boot_info *)(uintptr_t)mbi2_finish(&bld);
}

int start_module(struct mbi *mbi, uint64_t phys_max) {
  if (((mbi->flags & MBI_FLAG_MODS) == 0) || (mbi->mods_count == 0)) {
    printf("No module to start.\n");
    return -1;
  }

  /* Reserve this much memory to build a MBI2 boot information. */
  const size_t mbi2_extra_space_needed = 16 << 10;
  const uint64_t extra_space_addr =
      mbi_relocate_modules(mbi, phys_max, mbi2_extra_space_needed);

  struct module mod = mbi_pop_module(mbi);
  assert((uintptr_t)mod.mod_start == mod.mod_start,
         "Module is beyond what we can access");

  // The command line of the module becomes the kernel command line.
  mbi->cmdline = mod.string;
  mbi->flags |= MBI_FLAG_CMDLINE;

  struct mbi2_header *mbi2_header = mbi2_header_find(
      (void *)(uintptr_t)mod.mod_start, mod.mod_end - mod.mod_start);
  struct mbi2_boot_info *mbi2_info = NULL;

  if (mbi2_header) {
    printf("Found MBI2 header. Booting as MBI2.\n");
    mbi2_info =
        convert_mb1_to_mbi2(mbi, extra_space_addr, mbi2_extra_space_needed);
  } else {
    printf("Found no MBI2 header. Booting as MBI1.\n");
  }

  struct eh *elf = (struct eh *)(uintptr_t)mod.mod_start;
  assert(memcmp(elf->e_ident, ELFMAG, SELFMAG) == 0, "ELF header incorrect");

  // We assume that memory is available at the location where the BIOS would
  // usually execute boot blocks.
  uint8_t *code = (uint8_t *)0x7C00;

#define LOADER(EH, PH)                                                         \
  {                                                                            \
    struct EH *elfc = (struct EH *)elf;                                        \
    assert(                                                                    \
        elfc->e_type == 2 &&                                                   \
            ((elfc->e_machine == EM_386) || (elfc->e_machine == EM_X86_64)) && \
            elfc->e_version == 1,                                              \
        "ELF type incorrect");                                                 \
    assert(sizeof(struct PH) <= elfc->e_phentsize, "e_phentsize too small");   \
                                                                               \
    for (unsigned i = 0; i < elfc->e_phnum; i++) {                             \
      struct PH *ph = (struct PH *)(uintptr_t)(mod.mod_start + elfc->e_phoff + \
                                               i * elfc->e_phentsize);         \
      if (ph->p_type != 1)                                                     \
        continue;                                                              \
      gen_elf_segment(&code, ph->p_paddr,                                      \
                      (void *)(uintptr_t)(mod.mod_start + ph->p_offset),       \
                      ph->p_filesz, ph->p_memsz - ph->p_filesz);               \
    }                                                                          \
                                                                               \
    gen_mov(&code, EDX, elfc->e_entry);                                        \
  }

  switch (elf->e_ident[EI_CLASS]) {
  case ELFCLASS32:
    LOADER(eh, ph);
    break;
  case ELFCLASS64:
    LOADER(eh64, ph64);
    break;
  default:
    assert(false, "Invalid ELF class");
  }

  gen_mov(&code, EAX, mbi2_header ? MBI2_MAGIC : MBI_MAGIC);
  gen_jmp_edx(&code);

  assert(!mbi2_header || mbi2_info, "MBI2 info missing");
  asm volatile("jmp *%%edx"
               : /* No outputs */
               : "a"(0), "d"(0x7C00),
                 "b"(mbi2_header ? (uintptr_t)mbi2_info : (uintptr_t)mbi));

  __builtin_unreachable();
}

/* EOF */
