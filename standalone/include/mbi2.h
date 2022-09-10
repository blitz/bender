/*
 * Multiboot 2 structures
 *
 * Copyright (C) 2017, Alexander Boettcher <alexander.boettcher@genode-labs.com>
 *
 * This file is part of Bender.
 *
 * Bender is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Bender is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <util.h>

enum mbi2_enum {
  MBI2_HEADER_MAGIC = 0xe85250d6,

  /* The MBI2 header must be within this many bytes in a file. */
  MBI2_HEADER_BYTES = 32768,

  MBI2_MAGIC = 0x36d76289,
  MBI2_TAG_END = 0,
  MBI2_TAG_CMDLINE = 1,
  MBI2_TAG_MODULE = 3,
  MBI2_TAG_MEMORY = 6,
  MBI2_TAG_FB = 8,
  MBI2_TAG_EFI_32 = 11,
  MBI2_TAG_EFI_64 = 12,
  MBI2_TAG_RSDP_V1 = 14,
  MBI2_TAG_RSDP_V2 = 15,
  MBI2_TAG_EFI_MAP = 17,
  MBI2_TAG_EFI_IMAGE_32 = 19,
  MBI2_TAG_EFI_IMAGE_64 = 20,
};

struct mbi2_header {
  uint32_t magic;
  uint32_t architecture;
  uint32_t header_length;
  uint32_t checksum;
};
_Static_assert(sizeof(struct mbi2_header) == 16);

static inline uint32_t mbi2_header_checksum(const struct mbi2_header *h) {
  return h->magic + h->architecture + h->header_length + h->checksum;
}

struct mbi2_header *mbi2_header_find(void *data, size_t len);

struct mbi2_boot_info {
  uint32_t total_size;
  uint32_t reserved;
};
_Static_assert(sizeof(struct mbi2_boot_info) == 8);

/*
 * Each entry of the Multiboot2 information structure is a tag. This struct is
 * the header of each tag.
 *
 * To get to the payload, use mbi2_tag_payload.
 */
struct mbi2_tag {
  uint32_t type;
  uint32_t size;
};
_Static_assert(sizeof(struct mbi2_tag) == 8);

/*
 * Returns the payload of a tag.
 *
 * The returned pointer needs to be cast to the correct structure type depending
 * on the tag type. The payload types are below.
 */
static inline void *mbi2_tag_payload(struct mbi2_tag *tag) {
  /* The payload is located directly after the tag. */
  return tag + 1;
}

struct mbi2_module {
  uint32_t mod_start;
  uint32_t mod_end;
  char string[];
};
_Static_assert(sizeof(struct mbi2_module) == 8);

struct mbi2_memory {
  uint32_t entry_size;
  uint32_t entry_version;
};
_Static_assert(sizeof(struct mbi2_memory) == 8);

struct mbi2_memory_entry {
  uint64_t addr;
  uint64_t len;
  uint32_t type;
  uint32_t reserved;
};
_Static_assert(sizeof(struct mbi2_memory_entry) == 24);

/**
 * Returns the first memory map entry from the memory map.
 */
static inline struct mbi2_memory_entry *
mbi2_memory_map_first(struct mbi2_tag *mem_tag) {
  assert(mem_tag->type == MBI2_TAG_MEMORY,
         "Need memory tag, but got something else");
  struct mbi2_memory *mem = mbi2_tag_payload(mem_tag);

  return (struct mbi2_memory_entry *)(mem + 1);
}

/**
 * Returns the next memory map entry.
 */
static inline struct mbi2_memory_entry *
mbi2_memory_map_next(struct mbi2_tag *mem_tag,
                     struct mbi2_memory_entry *mem_entry) {
  assert(mem_tag->type == MBI2_TAG_MEMORY,
         "Need memory tag, but got something else");
  struct mbi2_memory *mem = mbi2_tag_payload(mem_tag);
  struct mbi2_memory_entry *end =
      (struct mbi2_memory_entry *)((char *)mem_tag + mem_tag->size);
  struct mbi2_memory_entry *next =
      (struct mbi2_memory_entry *)((char *)mem_entry + mem->entry_size);

  return next < end ? next : NULL;
}

static inline struct mbi2_tag *mbi2_tag_first(struct mbi2_boot_info *mbi) {
  return (struct mbi2_tag *)((char *)mbi + sizeof(*mbi));
}

static inline struct mbi2_tag *mbi2_tag_next(struct mbi2_tag *c) {
  size_t offset = (c->size + (sizeof(*c) - 1)) & ~(sizeof(*c) - 1);
  struct mbi2_tag *n = (struct mbi2_tag *)((char *)c + offset);
  return (n->type == 0) ? 0 : n;
}

struct mbi2_builder {
  char *start;
  char *cur;
  char *end;
};

/**
 * Start building a Multiboot2 info structure.
 */
struct mbi2_builder mbi2_build(uint64_t phys_addr, size_t max_size);

void mbi2_add_boot_cmdline(struct mbi2_builder *bld, const char *cmdline);
void mbi2_add_module(struct mbi2_builder *bld, uint64_t mod_addr,
                     uint64_t mod_end, const char *cmdline);

struct memory_map;
void mbi2_add_mbi1_memmap(struct mbi2_builder *bld,
                          struct memory_map *mbi1_mmap, uint32_t mbi1_mmap_end);

/**
 * Finishes the construction of the Multiboot2 info and returns the physical
 * address of its start.
 */
uint64_t mbi2_finish(struct mbi2_builder *bld);

/* EOF */
