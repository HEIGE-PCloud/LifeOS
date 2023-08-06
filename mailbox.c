#include "gpio.h"

// mailbox message buffer, 16 bit alligned
volatile unsigned int __attribute__((aligned(16))) mailbox[36];

#define MAILBOX_BASE (MMIO_BASE + 0x0000B880)
#define MAILBOX_READ ((volatile unsigned int*)(MAILBOX_BASE + 0x0))
#define MAILBOX_STATUS ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE ((volatile unsigned int*)(MAILBOX_BASE + 0x20))
#define MAILBOX_RESPONSE 0x80000000
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

// Make a mailbox call. Returns 0 on failure, non-zero on success
int mailbox_call(unsigned char channel) {
  unsigned int data =
      (((unsigned int)((unsigned long)&mailbox) & ~0xF) | (channel & 0xF));
  // Wait to write
  do {
    asm volatile("nop");
  } while (*MAILBOX_STATUS & MAILBOX_FULL);
  // Write data
  *MAILBOX_WRITE = data;
  // Wait for response
  while (1) {
    // Wait to read
    do {
      asm volatile("nop");
    } while (*MAILBOX_STATUS & MAILBOX_EMPTY);
    // Read response
    if (data == *MAILBOX_READ) {
      return mailbox[1] == MAILBOX_RESPONSE;
    }
  }
  return 0;
}