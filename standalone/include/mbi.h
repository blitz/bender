/*
 * Multiboot structures
 *
 * Copyright (C) 2006, Bernhard Kauer <kauer@os.inf.tu-dresden.de>
 * Copyright (C) 2009-2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
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

enum mbi_enum {
  MBI_MAGIC = 0x2badb002,
  MBI_FLAG_MEM = 1 << 0,
  MBI_FLAG_CMDLINE = 1 << 2,
  MBI_FLAG_MODS = 1 << 3,
  MBI_FLAG_MMAP = 1 << 6,
  MBI_FLAG_BOOT_LOADER_NAME = 1 << 9,
};

struct mbi {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t dummy0[4];
  uint32_t mmap_length;
  uint32_t mmap_addr;
  uint32_t dummy1[3];
  uint32_t boot_loader_name;
};

struct module {
  uint32_t mod_start;
  uint32_t mod_end;
  uint32_t string;
  uint32_t reserved;
};

typedef struct memory_map {
  uint32_t size;
  uint32_t base_addr_low;
  uint32_t base_addr_high;
  uint32_t length_low;
  uint32_t length_high;
  uint32_t type;
} memory_map_t;

static inline uint64_t mbi_memory_base_addr(memory_map_t *mmap) {
  return (uint64_t)mmap->base_addr_high << 32 | mmap->base_addr_low;
}

static inline uint64_t mbi_memory_length(memory_map_t *mmap) {
  return (uint64_t)mmap->length_high << 32 | mmap->length_low;
}

static inline memory_map_t *mbi_memory_map_next(memory_map_t *mmap,
                                                uint32_t mmap_end) {
  memory_map_t *next =
      (memory_map_t *)(mmap->size + (uintptr_t)mmap + sizeof(mmap->size));

  return (uintptr_t)next < mmap_end ? next : NULL;
}

enum memory_map_type {
  MMAP_AVAILABLE = 1,
};

/* EOF */
