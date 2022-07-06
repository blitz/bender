/* -*- Mode: C -*- */

#include <stdint.h>
#include <util.h>
#include <mbi.h>
#include <version.h>

static void
parse_cmdline(const char *cmdline)
{
  /* Nothing here yet. */
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

  printf("Our job is done. Sleeping.\n");
  cli_halt();
}
