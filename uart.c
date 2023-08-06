#include "gpio.h"

#define AUX_ENABLE (MMIO_BASE + 0x00215004)
#define AUX_MU_IO (MMIO_BASE + 0x00215040)
#define AUX_MU_IER (MMIO_BASE + 0x00215044)
#define AUX_MU_IIR (MMIO_BASE + 0x00215048)
#define AUX_MU_LCR (MMIO_BASE + 0x0021504C)
#define AUX_MU_MCR (MMIO_BASE + 0x00215050)
#define AUX_MU_LSR (MMIO_BASE + 0x00215054)
#define AUX_MU_MSR (MMIO_BASE + 0x00215058)
#define AUX_MU_SCRATCH (MMIO_BASE + 0x0021505C)
#define AUX_MU_CNTL (MMIO_BASE + 0x00215060)
#define AUX_MU_STAT (MMIO_BASE + 0x00215064)
#define AUX_MU_BAUD (MMIO_BASE + 0x00215068)

static inline void delay(int count) {
  for (int i = 0; i < count; i++) {
    asm volatile("nop");
  }
}

static inline void mmio_write(long reg, unsigned int val) {
  *(volatile unsigned int *)reg = val;
}

static inline unsigned int mmio_read(long reg) {
  return *(volatile unsigned int *)reg;
}

void init_uart() {
  mmio_write(AUX_ENABLE, 1);
  mmio_write(AUX_MU_IER, 0);
  mmio_write(AUX_MU_CNTL, 0);
  mmio_write(AUX_MU_LCR, 3);
  mmio_write(AUX_MU_MCR, 0);
  mmio_write(AUX_MU_IER, 0);
  mmio_write(AUX_MU_IIR, 0xC6);
  mmio_write(AUX_MU_BAUD, 270);
  register unsigned int res = mmio_read(GPFSEL1);
  res &= ~((7 << 12) | (7 << 15));
  res |= (2 << 12) | (2 << 15);
  mmio_write(GPFSEL1, res);
  mmio_write(GPPUD, 0);
  delay(200);
  mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
  delay(200);
  mmio_write(GPPUDCLK0, 0);
  mmio_write(AUX_MU_CNTL, 3);
}

void putc(unsigned int c) {
  do {
    asm volatile("nop");
  } while (!(mmio_read(AUX_MU_LSR) & 0x20));
  mmio_write(AUX_MU_IO, c);
}

char getc() {
  do {
    asm volatile("nop");
  } while (!(mmio_read(AUX_MU_LSR) & 0x01));
  char ch = (char)mmio_read(AUX_MU_IO);
  return ch == '\r' ? '\n' : ch;
}

void puts(char *s) {
  while (*s) {
    if (*s == '\n') {
      putc('\r');
    }
    putc(*s++);
  }
  putc('\r');
  putc('\n');
}

void print_hex(unsigned int d) {
  unsigned int n;
  for (int c = 28; c >= 0; c -= 4) {
    n = (d >> c) & 0xF;
    n += n > 9 ? 0x37 : 0x30;
    putc(n);
  }
}
