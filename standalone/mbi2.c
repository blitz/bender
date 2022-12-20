/* -*- Mode: C -*- */

#include <mbi2.h>

#include <acpi.h>
#include <mbi.h>
#include <util.h>

struct mbi2_header *mbi2_header_find(void *data, size_t len) {
  char *end = (char *)data + len;

  for (char *cur = data; cur + sizeof(struct mbi2_header) <= end; cur += 8) {
    struct mbi2_header *header = (struct mbi2_header *)cur;

    if (header->magic == MBI2_HEADER_MAGIC &&
        mbi2_header_checksum(header) == 0) {
      return header;
    }
  }

  return NULL;
}

enum {
  MBI2_TAG_ALIGNMENT = 8,
};

static size_t mbi2_available_bytes(struct mbi2_builder *bld) {
  return bld->end - bld->cur;
}

static void mbi2_add_blob(struct mbi2_builder *bld, size_t align,
                          const void *data, size_t size) {
  /* This is slightly too conservative, because we always assume that we need
   * the full alignment space. */
  assert(size + align <= mbi2_available_bytes(bld),
         "Out of space when building MBI2 info");

  size_t misaligned_bytes = (uintptr_t)bld->cur % align;
  size_t alignment_needed =
      misaligned_bytes == 0 ? 0 : (align - misaligned_bytes);

  bld->cur += alignment_needed;

  memcpy(bld->cur, data, size);

  bld->cur += size;
}

struct mbi2_builder mbi2_build(uint64_t phys_addr, size_t max_size) {
  assert((uintptr_t)phys_addr == phys_addr,
         "MBI2 can only be built in 32-bit accessible memory");

  /* We could relax this by handing out an aligned address in mbi2_finish. */
  assert((uintptr_t)phys_addr % MBI2_TAG_ALIGNMENT == 0,
         "The MBI2 info start address needs to be aligned");

  char *start_ptr = (void *)(uintptr_t)phys_addr;

  struct mbi2_builder bld = {
      .start = start_ptr,
      .cur = start_ptr,
      .end = start_ptr + max_size,
  };

  struct mbi2_boot_info info = {
      /* Will be filled out by mbi2_finish. */
      .total_size = 0,
      .reserved = 0,
  };

  mbi2_add_blob(&bld, MBI2_TAG_ALIGNMENT, &info, sizeof(info));

  return bld;
}

void mbi2_add_boot_cmdline(struct mbi2_builder *bld, const char *cmdline) {
  /* We have to account for the final NUL byte. */
  size_t cmdline_len = strlen(cmdline) + 1;

  struct mbi2_tag tag = {
      .type = MBI2_TAG_CMDLINE,
      .size = sizeof(struct mbi2_tag) + cmdline_len,
  };

  mbi2_add_blob(bld, MBI2_TAG_ALIGNMENT, &tag, sizeof(tag));
  mbi2_add_blob(bld, 1, cmdline, cmdline_len);
}

void mbi2_add_module(struct mbi2_builder *bld, uint64_t mod_addr,
                     uint64_t mod_end, const char *cmdline) {
  /* We have to account for the final NUL byte. */
  size_t cmdline_len = strlen(cmdline) + 1;

  struct mbi2_module mod = {
      .mod_start = mod_addr,
      .mod_end = mod_end,
  };

  struct mbi2_tag tag = {
      .type = MBI2_TAG_MODULE,
      .size =
          sizeof(struct mbi2_tag) + sizeof(struct mbi2_module) + cmdline_len,
  };

  mbi2_add_blob(bld, MBI2_TAG_ALIGNMENT, &tag, sizeof(tag));
  mbi2_add_blob(bld, 1, &mod, sizeof(mod));
  mbi2_add_blob(bld, 1, cmdline, cmdline_len);
}

void mbi2_add_mbi1_memmap(struct mbi2_builder *bld,
                          struct memory_map *mbi1_mmap,
                          uint32_t mbi1_mmap_end) {
  size_t mmap_count = 0;

  for (memory_map_t *cur = mbi1_mmap; cur != NULL;
       cur = mbi_memory_map_next(cur, mbi1_mmap_end)) {
    mmap_count += 1;
  }

  struct mbi2_tag tag = {
      .type = MBI2_TAG_MEMORY,
      .size = sizeof(struct mbi2_tag) + sizeof(struct mbi2_memory) +
              sizeof(struct mbi2_memory_entry) * mmap_count,
  };

  struct mbi2_memory mem = {
      .entry_size = sizeof(struct mbi2_memory_entry),
      .entry_version = 0,
  };

  mbi2_add_blob(bld, MBI2_TAG_ALIGNMENT, &tag, sizeof(tag));
  mbi2_add_blob(bld, 1, &mem, sizeof(mem));

  for (memory_map_t *cur = mbi1_mmap; cur != NULL;
       cur = mbi_memory_map_next(cur, mbi1_mmap_end)) {
    struct mbi2_memory_entry mem_entry = {
        .addr = mbi_memory_base_addr(cur),
        .len = mbi_memory_length(cur),
        .type = cur->type,
        .reserved = 0,
    };

    mbi2_add_blob(bld, 1, &mem_entry, sizeof(mem_entry));
  }
}

void mbi2_add_rsdp(struct mbi2_builder *bld, struct rsdp *rsdp) {
  /* We have to account for the final NUL byte. */
  size_t rsdp_len = rsdp->rev == 1 ? 20 : rsdp->size;

  struct mbi2_tag tag = {
    .type = rsdp->rev == 1 ? MBI2_TAG_RSDP_V1 : MBI2_TAG_RSDP_V2,
    .size = sizeof(struct mbi2_tag) + rsdp_len,
  };

  mbi2_add_blob(bld, MBI2_TAG_ALIGNMENT, &tag, sizeof(tag));
  mbi2_add_blob(bld, 1, rsdp, rsdp_len);
}

uint64_t mbi2_finish(struct mbi2_builder *bld) {
  struct mbi2_tag tag = {
      .type = MBI2_TAG_END,
      .size = sizeof(struct mbi2_tag),
  };

  mbi2_add_blob(bld, MBI2_TAG_ALIGNMENT, &tag, sizeof(tag));

  struct mbi2_boot_info *info = (struct mbi2_boot_info *)(bld->start);

  info->total_size = bld->cur - bld->start;

  return (uintptr_t)(bld->start);
}
