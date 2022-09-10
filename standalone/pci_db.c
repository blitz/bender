/* -*- Mode: C -*- */

#include <pci_db.h>

#define WILDCARD 0xFFFF

/* This list is searched from top to bottom. Wildcards always
   match. */
static const struct pci_db_entry vendor_db[] = {
    {0x1c00, 0x3253, QUIRK_SERIAL_BAR0_OFFSET_C0,
     " WCH CH353 Serial Controller"},

    /* Unknown (don't remove this item) */
    {WILDCARD, WILDCARD, NO_QUIRKS, "Unknown Device"}};

const struct pci_db_entry *pci_lookup_device(uint16_t vendor_id,
                                             uint16_t device_id) {
  const struct pci_db_entry *db = vendor_db;

  /* And they say that Lisp has a lot of parens... */
  while (!(((db->vendor_id == WILDCARD) || (db->vendor_id == vendor_id)) &&
           ((db->device_id == WILDCARD) || (db->device_id == device_id)))) {
    db++;
  }

  return db;
}

/* EOF */
