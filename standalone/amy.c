/* -*- Mode: C -*- */

#include <stdint.h>
#include <util.h>
#include <mbi.h>
#include <version.h>
#include <serial.h>
#include <bda.h>

static void
parse_cmdline(const char *cmdline)
{
  /* Nothing here yet. */
}

static void
probe_legacy(void)
{
  struct bios_data_area *bda = get_bios_data_area();

  for (size_t i = 0; i < ARRAY_SIZE(bda->com_port); i++) {
    uint16_t port = bda->com_port[i];

    /* At least on Qemu unused COM ports have a port number of 0. Also misaligned port numbers are bogus. */
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

  printf("\nAmy %s", version_str);
  printf("Probing for serial controllers...\n");

  probe_legacy();

  printf("Our job is done. Sleeping.\n");
  cli_halt();
}
