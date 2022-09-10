/* -*- Mode: C -*- */
/*
 * PCI database.
 *
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

#include <stdint.h>

struct pci_db_entry {
  uint16_t vendor_id;
  uint16_t device_id;
  uint32_t quirks;
  const char *device_name;
};

enum controller_quirks {
  NO_QUIRKS = 0,

  /* The device's serial ports are not at the start of BAR0, but at an offset of
     0xC0. */
  QUIRK_SERIAL_BAR0_OFFSET_C0 = 1 << 0,

  /* ... */
};

const struct pci_db_entry *pci_lookup_device(uint16_t vendor_id,
                                             uint16_t device_id);

/* EOF */
