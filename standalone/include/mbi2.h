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

enum mbi2_enum
  {
    MBI2_MAGIC            = 0x36d76289,
    MBI2_TAG_END          = 0,
    MBI2_TAG_CMDLINE      = 1,
    MBI2_TAG_MODULE       = 3,
    MBI2_TAG_MEMORY       = 6,
    MBI2_TAG_FB           = 8,
    MBI2_TAG_EFI_32       = 11,
    MBI2_TAG_EFI_64       = 12,
    MBI2_TAG_RSDP_V1      = 14,
    MBI2_TAG_RSDP_V2      = 15,
    MBI2_TAG_EFI_MAP      = 17,
    MBI2_TAG_EFI_IMAGE_32 = 19,
    MBI2_TAG_EFI_IMAGE_64 = 20,
  };

struct mbi2_boot_info
{
  uint32_t total_size;
  uint32_t reserved;
};
_Static_assert(sizeof(struct mbi2_boot_info) == 8);

struct mbi2_tag
{
  uint32_t type;
  uint32_t size;
};
_Static_assert(sizeof(struct mbi2_tag) == 8);

struct mbi2_module
{
  uint32_t mod_start;
  uint32_t mod_end;
  char     string[];
};
_Static_assert(sizeof(struct mbi2_module) == 8);

struct mbi2_memory
{
  uint64_t addr;
  uint64_t len;
  uint32_t type;
  uint32_t reserved;
};
_Static_assert(sizeof(struct mbi2_memory) == 24);

static inline
struct mbi2_tag * mbi2_tag_first(struct mbi2_boot_info *mbi)
{
  return (struct mbi2_tag *)((char *)mbi + sizeof(*mbi));
}

static inline
struct mbi2_tag * mbi2_tag_next(struct mbi2_tag *c)
{
  size_t offset = (c->size + (sizeof(*c) - 1)) & ~(sizeof(*c) - 1);
  struct mbi2_tag * n = (struct mbi2_tag *)((char *)c + offset);
  return (n->type == 0) ? 0 : n;
}

/* EOF */
