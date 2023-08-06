#include "framebuffer.h"
#include "life.h"
#include "mailbox.h"
#include "positions.h"
#include "timer.h"
#include "uart.h"
#define SCREEN_WIDTH 192
#define SCREEN_HEIGHT 107

int main(void) {
  init_uart();
  init_framebuffer();
  init_life(SCREEN_WIDTH, SCREEN_HEIGHT, 10);
  start_state_t *states[] = {&spaceship, &spider,   &glider_gun,
                             &weekender, &snail,    &bandersnatch,
                             &scholar,   &sir_robin};
  while (1) {
    for (int i = 0; i < (sizeof(states) / sizeof(start_state_t *)); i++) {
      init_cells(states[i]);
      print_cells();
      draw_string(0, 1072, states[i]->name, BRIGHT_WHITE);
      draw_string(112, 1072, "life count:", BRIGHT_WHITE);
      for (int j = 0; j < states[i]->max_ticks; j++) {
        next_tick();
        draw_number(get_alive_count(), 200, 1072, BRIGHT_WHITE);
      }
      clear_life();
      draw_string(0, 1072, states[i]->name, 0x0);
    }
  }
}
