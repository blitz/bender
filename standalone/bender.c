/* -*- Mode: C -*- */

#include <pci.h>
#include <mbi.h>
#include <util.h>
#include <elf.h>
#include <version.h>
#include <serial.h>
#include <bda.h>

/*
 * Configuration (set by command line parser)
 */

/* If true, accepts any device with SIMPLE_COMM class, not just serial controllers. This might do the wrong thing. */
static bool be_promisc = false;

/* Don't look for PCI serial controllers, if the BIOS indicates that we have legacy COM ports. */
static bool accept_legacy = false;

/* Upper limit of where we are willing to relocate modules. */
static uint64_t phys_max_relocate = 1ULL << 31; /* below 2G */

static void
parse_cmdline(const char *cmdline)
{
  char *last_ptr = NULL;
  char cmdline_buf[256];
  char *token;
  unsigned i;

  strncpy(cmdline_buf, cmdline, sizeof(cmdline_buf));

  for (token = strtok_r(cmdline_buf, " ", &last_ptr), i = 0;
       token != NULL;
       token = strtok_r(NULL, " ", &last_ptr), i++) {

    /* Our name is not interesting. */
    if (i == 0)
      continue;

    if (strcmp(token, "promisc") == 0) {
      be_promisc = true;
    } else if (strcmp(token, "phys_max=256M") == 0) {
      phys_max_relocate = 256ULL * 1024 * 1024;
    } else if (strcmp(token, "accept_legacy") == 0) {
      accept_legacy = true;
    }
  }
}

static uint16_t apply_quirks(struct pci_device *pcid, uint16_t raw_iobase)
{
  uint16_t iobase = raw_iobase;
  if (pcid->db->quirks & QUIRK_SERIAL_BAR0_OFFSET_C0) {
    printf("Found XR16850 chip. Adding 0xc0 to iobase offset.\n");
    iobase += 0xc0;
  }

  return iobase;
}

static bool
is_serial_controller(struct pci_device *dev)
{
  return pci_matches_class(dev, PCI_CLASS_SIMPLE_COMM, PCI_SUBCLASS_SERIAL_CTRL);
}

static bool
is_simple_comm(struct pci_device *dev)
{
  return pci_matches_class(dev, PCI_CLASS_SIMPLE_COMM, PCI_SUBCLASS_ANY);
}

int
main(uint32_t magic, struct mbi *mbi)
{
  if (magic == MBI_MAGIC) {
    if ((mbi->flags & MBI_FLAG_CMDLINE) != 0)
      parse_cmdline((const char *)mbi->cmdline);
  } else {
    printf("Not loaded by Multiboot-compliant loader. Bye.\n");
    return 1;
  }

  printf("\nBender %s", version_str);
  printf("Looking for serial controllers on the PCI bus...\n");

  printf("Promisc is %s.\n", be_promisc ? "on" : "off");

  uint16_t *com0_port      = (uint16_t *)(get_bios_data_area());
  uint16_t *equipment_word = &get_bios_data_area()->equipment;

  if (accept_legacy && ((*equipment_word & 0x7) != 0)) {
    printf("Found legacy serial. Skipping PCI discovery.\n");
    goto boot_next;
  }

  /* Try to find the last matching controller. */
  bool found_ctrl = false;
  bool found_matching_ctrl = false;
  struct pci_iter iter = PCI_ITER_INIT;
  struct pci_device dev;
  struct pci_device serial_ctrl;

  while ((found_ctrl = pci_iter_next_matching(&iter, be_promisc ? is_simple_comm : is_serial_controller, &dev))) {
    found_matching_ctrl = true;
    memcpy(&serial_ctrl, &dev, sizeof(serial_ctrl));
  }

  if (found_matching_ctrl) {
    printf("  found at %x.\n", serial_ctrl.cfg_address);
  } else {
    printf("  none found.\n");
    goto boot_next;
  }

  uint16_t iobase = 0;
  bool has_iobar = pci_first_iobar_base(&serial_ctrl, &iobase);

  if (!has_iobar) {
    printf("Controller has no PIO BAR.\n");
    goto boot_next;
  }

  iobase = apply_quirks(&serial_ctrl, iobase);

  if (iobase != 0) {
    printf("Patching BDA with I/O port 0x%x.\n", iobase);
    *com0_port      = iobase;
    *equipment_word = (*equipment_word & ~(0xF << 9)) | (1 << 9); /* One COM port available */
  } else {
    printf("I/O ports for controller not found.\n");
  }

 boot_next:

  if (serial_ports(get_bios_data_area()))
    serial_init();

  printf("Bender: Hello World.\n");

  return start_module(mbi, false, phys_max_relocate);
}
