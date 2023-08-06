#include "timer.h"
#include "gpio.h"

#define SYSTMR_LO ((volatile unsigned int*)(MMIO_BASE + 0x00003004))
#define SYSTMR_HI ((volatile unsigned int*)(MMIO_BASE + 0x00003008))

void wait(unsigned long long msec) {
  register unsigned long long freq = 0;
  register unsigned long long cnt = 0;
  register unsigned long long res = 0;
  // get the current counter frequency
  asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
  // read the current counter
  asm volatile("mrs %0, cntpct_el0" : "=r"(cnt));
  // calculate required count increase
  unsigned long i = ((freq / 1000) * msec) / 1000;
  // loop while counter increase is less than i
  do {
    asm volatile("mrs %0, cntpct_el0" : "=r"(res));
  } while (res - cnt < i);
}

unsigned long get_counter() {
  unsigned long cnt = 0;
  asm volatile("mrs %0, cntpct_el0" : "=r"(cnt));
  return cnt;
}

unsigned long get_frequency() {
  unsigned long freq = 0;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
  return freq;
}

