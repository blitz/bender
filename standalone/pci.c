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
  dev->vendor_id = vendor_id;
  dev->device_id = device_id;
}

static void
pci_iter_advance(struct pci_iter *iter)
{
  assert(iter->function <= iter->max_function, "Function index out of range");

  if (iter->function == iter->max_function) {
    iter->function = 0;
    iter->max_function = 0;
    iter->bus_device += 1;
  } else {
    iter->function += 1;
  }
}

bool
pci_iter_next_device(struct pci_iter *iter, struct pci_device *dev)
{
  while (iter->bus_device < (1 << 13)) {

    uint32_t addr = 0x80000000 | iter->bus_device << 11 | iter->function<<8;

    if (pci_read_uint32(addr + PCI_CFG_VENDOR_ID) == 0xFFFFFFFF) {
      pci_iter_advance(iter);
      continue;
    }

    /* Is it a multifunction device? If so, we need to probe its functions. */
    if (!iter->max_function && pci_read_uint8(addr + 14) & 0x80) {
      iter->max_function = 7;
    }

    populate_device_info(addr, dev);
    pci_iter_advance(iter);
    return true;
  }

  return false;
}

bool
pci_iter_next_matching(struct pci_iter *iter, bool(*predicate)(struct pci_device *),
                            struct pci_device *dev)
{
  while (pci_iter_next_device(iter, dev)) {
    if (predicate(dev)) {
      return true;
    }
  }

  return false;
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

bool pci_matches_class(struct pci_device *dev, uint8_t class, uint8_t subclass)
{
  uint16_t full_class = class << 8 | subclass;
  uint16_t class_mask = (subclass == PCI_SUBCLASS_ANY) ? 0xFF00 : 0xFFFF;

  return (full_class & class_mask)
    == ((pci_read_uint32(dev->cfg_address + PCI_CFG_REVID) >> 16) & class_mask);
}

/* EOF */
