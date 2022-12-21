/* -*- Mode: C -*- */

#include <acpi.h>
#include <bda.h>
#include <stddef.h>
#include <util.h>

/* ACPI code inspired by Vancouver. */

/**
 * Calculate the ACPI checksum of a table.
 */
char acpi_checksum(const char *table, size_t count) {
  char res = 0;
  while (count--)
    res += *(table++);
  return res;
}

/**
 * Return the rsdp.
 */
const struct rsdp *acpi_get_rsdp(void) {
  __label__ done;
  const struct rsdp *ret = 0;

  void check(char *p) {
    if ((memcmp(p, "RSD PTR ", 8) == 0) && (acpi_checksum(p, 20) == 0)) {
      ret = (struct rsdp *)p;
      goto done;
    }
  }

  void find(uintptr_t start, size_t len) {
    for (uintptr_t cur = start; cur < start + len; cur += 16)
      check((char *)cur);
  }

  find((uintptr_t)get_bios_data_area(), 0x400); /* BDA */
  find(0xe0000, 0x20000);                       /* BIOS read-only memory */

done:
  return ret;
}

/* EOF */
