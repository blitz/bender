/* From Nova. GPL'd */
#include <asm.h>
#include <serial.h>
#include <bda.h>
#include <util.h>

enum Port  {
  THR         = 0,    // Transmit Holding Register    (write)
  IER         = 1,    // Interrupt Enable Register    (write)
  FCR         = 2,    // FIFO Control Register        (write)
  LCR         = 3,    // Line Control Register        (write)
  MCR         = 4,    // Modem Control Register       (write)
  LSR         = 5,    // Line Status Register         (read)
  DLR_LOW     = 0,
  DLR_HIGH    = 1,
};

enum {
  FCR_FIFO_ENABLE     = 1u << 0,  // FIFO Enable
  FCR_RECV_FIFO_RESET = 1u << 1,  // Receiver FIFO Reset
  FCR_TMIT_FIFO_RESET = 1u << 2,  // Transmit FIFO Reset

  LCR_DATA_BITS_8     = 3u << 0,
  LCR_STOP_BITS_1     = 0u << 2,
  LCR_DLAB            = 1u << 7,

  MCR_DTR             = 1u << 0,  // Data Terminal Ready
  MCR_RTS             = 1u << 1,  // Request To Send

  LSR_TMIT_HOLD_EMPTY = 1u << 5,
};

static uint16_t serial_base;
static bool     output_enabled = false;

void
serial_send (int c)
{
  if (!output_enabled) return;

  unsigned max_tries = 0x10000;
  while (!(inb (serial_base + LSR) & LSR_TMIT_HOLD_EMPTY)) {
    asm volatile ("pause");
    if (max_tries-- == 0) {
      output_enabled = false;
      return;
    }
  }

  outb (serial_base + THR, c);
}

void
serial_init(void)
{
  // We could check whether COM1 is actually available via the equipment word in the BIOS Data Area, but this has not
  // been reliable. So now we just blindly configure the port and hope for the best.
  serial_init_port(get_bios_data_area()->com_port[0]);
}

void
serial_init_port(uint16_t base_port)
{
  output_enabled = true;
  serial_base = base_port;

  /* Programming the first serial adapter (8N1) */
  outb (serial_base + LCR, LCR_DLAB);

  /* 115200 baud */
  outb (serial_base + DLR_LOW,  1);
  outb (serial_base + DLR_HIGH, 0);

  /* 9600 baud */
  /*   outb (serial_base + DLR_LOW,  0x0C); */
  /*   outb (serial_base + DLR_HIGH, 0); */

  outb (serial_base + LCR, LCR_DATA_BITS_8 | LCR_STOP_BITS_1);
  outb (serial_base + IER, 0);
  outb (serial_base + FCR, FCR_FIFO_ENABLE | FCR_RECV_FIFO_RESET | FCR_TMIT_FIFO_RESET);
  outb (serial_base + MCR, MCR_DTR | MCR_RTS);
}

/* EOF */
