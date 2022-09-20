/* -*- Mode: C -*- */

#include <bda.h>
#include <mbi.h>
#include <mbi2.h>
#include <pci.h>
#include <serial.h>
#include <stdint.h>
#include <util.h>
#include <version.h>

static void parse_cmdline(const char *cmdline) { /* Nothing here yet. */
}

static void probe_legacy(void) {
  struct bios_data_area *bda = get_bios_data_area();

  for (size_t i = 0; i < ARRAY_SIZE(bda->com_port); i++) {
    uint16_t port = bda->com_port[i];

    /* At least on Qemu unused COM ports have a port number of 0. Also
     * misaligned port numbers are bogus. */
    if (port == 0 || ((port & 0x7) != 0)) {
      printf("Skipping COM%zu at port %x.\n", i + 1, port);
      continue;
    }

    serial_init_port(port);

    /* COM port numbers start at 1. */
    printf("Probing COM%zu at port %x...\n", i + 1, port);

    serial_disable();
  }
}

static bool is_serial_controller(struct pci_device *dev) {
  return pci_matches_class(dev, PCI_CLASS_SIMPLE_COMM,
                           PCI_SUBCLASS_SERIAL_CTRL);
}

static uint16_t apply_quirks(struct pci_device *pcid, uint16_t raw_iobase) {
  uint16_t iobase = raw_iobase;
  if (pcid->db->quirks & QUIRK_SERIAL_BAR0_OFFSET_C0) {
    printf("Found XR16850 chip. Adding 0xc0 to iobase offset.\n");
    iobase += 0xc0;
  }

  return iobase;
}

static void probe_pci(void) {
  struct pci_iter iter = PCI_ITER_INIT;
  struct pci_device dev;

  while (pci_iter_next_matching(&iter, is_serial_controller, &dev)) {
    printf("Found Serial Controller: %lx\n", dev.cfg_address);

    uint16_t iobase = 0;

    if (!pci_first_iobar_base(&dev, &iobase)) {
      printf("No PIO BAR. Skipping.\n");
      continue;
    }

    iobase = apply_quirks(&dev, iobase);

    serial_init_port(iobase);
    printf("Probing PCI serial controller %lx at port %x...\n", dev.cfg_address,
           iobase);
    serial_disable();
  }
}

int main(uint32_t magic, struct mbi *mbi) {
  if (magic == MBI_MAGIC) {
    if ((mbi->flags & MBI_FLAG_CMDLINE) != 0)
      parse_cmdline((const char *)mbi->cmdline);
  } else if (magic == MBI2_MAGIC) {
    printf("Multiboot2 is not supported. Please boot via Multiboot1. Bye.\n");
    return 1;
  } else {
    printf("Not loaded by Multiboot-compliant loader. Bye.\n");
    return 1;
  }

  printf("\nAmy %s", version_str);

  printf("Probing for legacy serial controllers...\n");
  probe_legacy();

  printf("Probing for PCI serial controllers...\n");
  probe_pci();

  printf("Our job is done. Sleeping.\n");
  cli_halt();
}
