/*
 * PCI functions.
 *
 * Copyright (C) 2006, Bernhard Kauer <kauer@os.inf.tu-dresden.de>
 * Copyright (C) 2009-2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
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

#include <stdint.h>
#include <stdbool.h>
#include <pci_db.h>

enum pci_class {
  PCI_CLASS_BRIDGE_DEV      = 0x06,
  PCI_CLASS_SIMPLE_COMM     = 0x07,
  PCI_CLASS_SERIAL_BUS_CTRL = 0x0C,
};

enum pci_subclass {
  PCI_SUBCLASS_IEEE_1394    = 0x00,
  PCI_SUBCLASS_HOST_BRIDGE  = 0x00,
  PCI_SUBCLASS_ISA_BRIDGE   = 0x01,
  PCI_SUBCLASS_PCI_BRIDGE   = 0x04,
  PCI_SUBCLASS_PCMCIA_BRIDGE  = 0x05,
  PCI_SUBCLASS_CARDBUS_BRIDGE = 0x07,
  PCI_SUBCLASS_SERIAL_CTRL  = 0x00,
  PCI_SUBCLASS_SERIAL_OTHER = 0x80,

  PCI_SUBCLASS_ANY          = 0xFF,
};

enum pci_config_space {
  PCI_CFG_VENDOR_ID = 0x0,
  PCI_CFG_REVID = 0x08,         /* Read uint32 to get class code in upper 16bit */
  PCI_CFG_BAR0  = 0x10,
  PCI_CFG_BAR1  = 0x14,
  PCI_CFG_BAR2  = 0x18,
  PCI_CFG_BAR3  = 0x1C,
};

enum pci_constants {
  PCI_ADDR_PORT = 0xcf8,
  PCI_DATA_PORT = 0xcfc,
  PCI_CONF_HDR_CMD = 4,
  PCI_CONF_HDR_CAP = 52,
  PCI_CAP_OFFSET = 1,
  PCI_CAP_ID_EXP   = 0x10,
  PCI_EXP_TYPE_PCI_BRIDGE=0x7,
  PCI_BAR_TYPE_MASK = 1U,
  PCI_BAR_TYPE_IO   = 1U,
  PCI_BAR_IO_MASK   = ~1U,

  PCI_BAR_NUM = 6,
};

struct pci_device {
  const struct pci_db_entry *db;
  uint32_t cfg_address;		/* Address of config space */

  uint16_t vendor_id;
  uint16_t device_id;
};

/// An iterator over PCI devices.
///
/// To use this, initialize it with PCI_ITER_INIT.
struct pci_iter {
  unsigned bus_device;
  uint8_t function;
  uint8_t max_function;
};

#define PCI_ITER_INIT { .bus_device = 0, .function = 0, .function = 0 }

/// Iterate to the next PCI device.
///
/// Returns true, if a device was found. In this case, *dev will be filled with device info.
bool pci_iter_next_device(struct pci_iter *iter, struct pci_device *dev);

/// Similar to pci_iter_next_device, but finds the next device that matches a predicate.
bool pci_iter_next_matching(struct pci_iter *iter, bool(*predicate)(struct pci_device *),
                            struct pci_device *dev);

/* Find a device by its class. Always finds the last device of the
   given class. On success, returns true and fills out the given
   pci_device structure. If subclass is 0xFF, it will be
   ignored. Otherwise, returns false. */
bool pci_find_device_by_class(uint8_t class, uint8_t subclass,
			      struct pci_device *dev);

/* Check whether a device matches a specific device class. */
bool pci_matches_class(struct pci_device *dev, uint8_t class, uint8_t subclass);

/// Find the I/O port of the first PIO BAR.
///
/// Returns true, if a PIO BAR is found. In this case, *iobase contains the port number.
bool pci_first_iobar_base(struct pci_device *dev, uint16_t *iobase);

/* EOF */
