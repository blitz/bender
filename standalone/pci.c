/*
 * \brief   DEV and PCI code.
 * \date    2006-10-25
 * \author  Bernhard Kauer <kauer@tudos.org>
 */
/* Based on fulda. Original copyright: */
/*
 * Copyright (C) 2006  Bernhard Kauer <kauer@tudos.org>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the OSLO package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */


#include <util.h>
#include <pci.h>

static uint8_t
pci_read_uint8(unsigned addr)
{
  outl(PCI_ADDR_PORT, addr);
  return inb(PCI_DATA_PORT + (addr & 3));
}

static uint32_t
pci_read_uint32(unsigned addr)
{
  outl(PCI_ADDR_PORT, addr);
  return inl(PCI_DATA_PORT);
}

static uint32_t
pci_cfg_read_uint32(const struct pci_device *dev, uint32_t offset)
{
  return pci_read_uint32(dev->cfg_address + offset);
}

static void
populate_device_info(uint32_t cfg_address, struct pci_device *dev)
{
  uint32_t vendor_id = pci_read_uint32(cfg_address + PCI_CFG_VENDOR_ID);
  uint32_t device_id = vendor_id >> 16;
  vendor_id &= 0xFFFF;

  dev->db = pci_lookup_device(vendor_id, device_id);
  dev->cfg_address = cfg_address;
}


bool
pci_find_device_by_class(uint8_t class, uint8_t subclass,
                         struct pci_device *dev)
{
  uint32_t res = 0;
  uint16_t full_class = class << 8 | subclass;
  uint16_t class_mask = (subclass == PCI_SUBCLASS_ANY) ? 0xFF00 : 0xFFFF;

  assert(dev != NULL, "Invalid dev pointer");

  for (unsigned i=0; i<1<<13; i++) {
    uint8_t maxfunc = 0;

    for (unsigned func = 0; func <= maxfunc; func++) {
      uint32_t addr = 0x80000000 | i<<11 | func<<8;

      if (!maxfunc && pci_read_uint8(addr+14) & 0x80)
        maxfunc=7;

      if ((full_class & class_mask) == ((pci_read_uint32(addr+0x8) >> 16) & class_mask))
        res = addr;
    }
  }

  if (res != 0) {
    populate_device_info(res, dev);
    return true;
  } else {
    return false;
  }
}

uint32_t pci_cfg_read_bar(struct pci_device *dev, unsigned bar_no)
{
  assert(bar_no < PCI_BAR_NUM, "Invalid BAR number");
  return pci_cfg_read_uint32(dev, PCI_CFG_BAR0 + 4*bar_no);
}

/* EOF */
