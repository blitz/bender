/* -*- Mode: C -*- */
/*
 * ACPI definitions.
 *
 * Copyright (C) 2009-2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
 * Copyright (C) 2009-2012, Bernhard Kauer <kauer@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of Morbo.
 *
 * Morbo is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Morbo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct rsdp {
  char signature[8];
  uint8_t checksum;
  char oem[6];
  uint8_t rev;
  uint32_t rsdt;
  uint32_t size;
  uint64_t xsdt;
  uint8_t ext_checksum;
  char _res[3];
} __attribute__((packed));
_Static_assert(sizeof(struct rsdp) == 36, "Invalid RSDP layout");

struct acpi_table {
  char signature[4];
  uint32_t size;
  uint8_t rev;
  uint8_t checksum;
  char oemid[6];
  char oemtabid[8];
  uint32_t oemrev;
  char creator[4];
  uint32_t crev;
} __attribute__((packed));

char acpi_checksum(const char *table, size_t count);

const struct rsdp *acpi_get_rsdp(void);

/* EOF */
