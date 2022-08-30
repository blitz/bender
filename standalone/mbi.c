/* -*- Mode: C -*- */

#include <mbi.h>
#include <stddef.h>
#include <util.h>

/** Find a sufficiently large block of free memory that is page aligned.
 */
static bool
mbi_find_memory(const struct mbi *multiboot_info, size_t len,
                void **block_start_out, size_t *block_len_out,
                bool highest, uint64_t const limit_to)
{
  bool found         = false;
  uint32_t mmap_end  = multiboot_info->mmap_addr + multiboot_info->mmap_length;

  /* be paranoid */
  if (limit_to <= len)
    return false;

  for (memory_map_t *mmap = (memory_map_t *)multiboot_info->mmap_addr;
       mmap != NULL;
       mmap = mbi_memory_map_next(mmap, mmap_end)) {

    uint64_t block_len  = (uint64_t)mmap->length_high<<32 | mmap->length_low;
    uint64_t block_addr = (uint64_t)mmap->base_addr_high<<32 | mmap->base_addr_low;

    /* Memory blocks may not be page aligned. Round length and address
       to page granularity. */
    uint64_t nblock_addr = (block_addr + 0xFFF) & ~0xFFF;
    if (nblock_addr > (block_addr + block_len)) continue;
    block_len -= (nblock_addr - block_addr);
    block_len  = block_len & ~0xFFF;
    block_addr = nblock_addr;

    if ((mmap->type == MMAP_AVAILABLE) && (block_len >= len) &&
        (block_addr <= limit_to - len)) {

      if ((found == true) && ((uintptr_t)*block_start_out > block_addr))
        continue;

      found = true;

      /* take care that (block_addr+len) is below limit_to */
      uint64_t top_addr = block_addr + block_len;
      if (top_addr > limit_to)
        top_addr = limit_to;

      *block_start_out = (void *)(uintptr_t)top_addr - len;
      *block_len_out   = (size_t)len;

      if (!highest) return true;
    }
  }

  return found;
}

/**
 * Push all modules to the highest available location in memory below phys_max.
 *
 * With extra_space additional memory behind the modules can be requested. The
 */
uint64_t
mbi_relocate_modules(struct mbi *mbi, uint64_t phys_max, size_t extra_space)
{
  size_t size = extra_space;

  struct module *mods = (struct module *)mbi->mods_addr;
  struct {
    size_t modlen;
    size_t slen;
  } minfo[mbi->mods_count];

  for (unsigned i = 0; i < mbi->mods_count; i++) {

    minfo[i].modlen = mods[i].mod_end - mods[i].mod_start;
    minfo[i].slen   = strlen((const char *)mods[i].string) + 1;

    size += minfo[i].modlen;
    size += minfo[i].slen;

    /* Round up to page size */
    size = (size + 0xFFF) & ~0xFFF;
  }

  void *block;
  size_t block_len;

  if (mbi_find_memory(mbi, size, &block, &block_len, true, phys_max)) {
    /* Check for overlap */
    uintptr_t reladdr = (uintptr_t)block + block_len - size;
    for (unsigned i = 0; i < mbi->mods_count; i++) {
      if (mods[i].mod_end > reladdr) {
        if (reladdr == mods[i].mod_start) {
          printf("Modules seem to be relocated. Good.\n");
          return (uint64_t)(uintptr_t)block;
        } else {
          printf("Modules might overlap.\nRelocate to %p, but module at %8x-%8x.\n",
                 reladdr, mods[i].mod_start, mods[i].mod_end-1);
          assert(false, "Failed to relocate");
        }
      }
    }

    printf("Need %8x bytes to relocate modules.\n", size);
    printf("Relocating to %8x: \n", reladdr);

    for (int i = mbi->mods_count - 1; i >= 0; i--) {
      size_t target_len = minfo[i].modlen;
      block_len -= (minfo[i].slen + target_len + 0xFFF) & ~0xFFF;

      printf("Copying %u bytes...\n", minfo[i].modlen);
      memcpy((char *)block + block_len, (void *)mods[i].mod_start,
             minfo[i].modlen);

      mods[i].mod_start = (size_t)((char *)block + block_len);
      mods[i].mod_end = mods[i].mod_start + target_len;

      memcpy((char *)block + block_len + target_len,
             (void *)mods[i].string, minfo[i].slen);
      mods[i].string = (uintptr_t)((char *)block + block_len + target_len);
    }
    printf("\n");

  } else {
    printf("Cannot relocate. Trying without...\n");

    if (!mbi_find_memory(mbi, extra_space, &block, &block_len, true, phys_max)) {
      assert(false, "Cannot find space for extra_space");
    }
  }

  assert(block_len >= extra_space, "We are left with less space than we should?");

  /* We copy modules from the end of our allocated region. The front of the block is thus the free extra_space. */
  return (uint64_t)(uintptr_t)block;
}

/**
 * Pop one module off the Multiboot1 info.
 *
 * Returns the physical address of the module we removed from the module list.
 */
uint64_t
mbi_pop_module(struct mbi *mbi)
{
  assert(mbi->mods_count > 0, "No module to start?");

  struct module *m  = (struct module *) mbi->mods_addr;
  mbi->mods_addr += sizeof(struct module);
  mbi->mods_count--;
  mbi->cmdline = m->string;

  // Switch on the command line, because we assume that m->string above is always initialized.
  mbi->flags |= MBI_FLAG_CMDLINE;

  return m->mod_start;
}

/* EOF */
