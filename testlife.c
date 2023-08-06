#include "framebuffer.h"
#include "life.h"
#include "mailbox.h"
#include "uart.h"
#define SCREEN_WIDTH 192
#define SCREEN_HEIGHT 108

int main(void) {
  init_uart();
  init_framebuffer();
  init_life(SCREEN_WIDTH, SCREEN_HEIGHT, 10);
  start_state_t state = {
      .len = 5,
      .name = "test",
      .pos = (pos_t[]){{1, 2}, {1, 3}, {2, 1}, {2, 2}, {3, 2}},
  };
  init_cells(&state);
  print_cells();
  next_tick();
  return 0;
}
