#include "life.h"

#include "framebuffer.h"
#include "uart.h"
#define LIFE_BASE_ADDR (void *)0x00500000
#define LIFE_BASE_ADD2 (void *)0x00900000
int life_width;
int life_height;
int cell_size;
#define BOARD_POS(board, x, y) (board + x * life_width + y)
int alive_count;
char *board_1;
char *board_2;
char *current_board;
int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};


// Initialises both boards with all cells dead/0
// taking w as width of board, h as height of board (in cells)
// and s as the side length of each cell (in pixels).
void init_life(int w, int h, int s) {
  board_1 = (char *)LIFE_BASE_ADDR;
  board_2 = (char *)(LIFE_BASE_ADD2);
  life_width = w;
  life_height = h;
  cell_size = s;
  for (int i = 0; i < life_width; i++) {
    for (int j = 0; j < life_height; j++) {
      *(char *)BOARD_POS(board_1, i, j) = 0;
      *(char *)BOARD_POS(board_2, i, j) = 0;
    }
  }
  current_board = board_1;
}

// Changes value of cell at position (x, y) on the current board
void set_cell(int x, int y, char value) {
  *(char *)BOARD_POS(current_board, x, y) = value;
}

// Gets value of cell at position (x, y) on the current board
char get_cell(int x, int y) { return *BOARD_POS(current_board, x, y); }

void init_cells(start_state_t *state) {
  for (int i = 0; i < state->len; i++) {
    if (state->pos[i].x < 0 || state->pos[i].x >= life_width ||
        state->pos[i].y < 0 || state->pos[i].y >= life_height) {
      puts("invalid position\n");
      continue;
    }
    set_cell(state->pos[i].x, state->pos[i].y, 1);
    alive_count++;
  }
}

void clear_life() {
  for (int i = 0; i < life_width; i++) {
    for (int j = 0; j < life_height; j++) {
      set_cell(i, j, 0);
    }
  }
  alive_count = 0;
}

// Changes the current board to the other (next) board
void swap_boards() {
  if (current_board == board_1) {
    current_board = board_2;
  } else {
    current_board = board_1;
  }
}

// Returns the next (not current) board
char *next_board() {
  if (current_board == board_1) {
    return board_2;
  } else {
    return board_1;
  }
}

// Changes value of cell at position (x, y) on the next board
void set_cell_next(int x, int y, char value) {
  *(char *)BOARD_POS(next_board(), x, y) = value;
}

// Counts number of live cells in 8 cells, directly
// adjacent or diagonal to the cell at position (x, y)
int count_neighbour(int x, int y) {
  int cnt = 0;
  for (int i = 0; i < 8; i++) {
    int xp = x + dx[i];
    int yp = y + dy[i];
    if (xp >= 0 && xp < life_width && yp >= 0 && yp < life_height &&
        get_cell(xp, yp) == 1) {
      cnt++;
    }
  }
  return cnt;
}


void draw_cell(int i, int j, int val) {
  draw_rect(i * cell_size, j * cell_size, i * cell_size + cell_size,
                    j * cell_size + cell_size, 7, val);
}

// Computes and sets next board to next state,
// while updating display for cells which changed state.
void next_tick() {
  for (int i = 0; i < life_width; i++) {
    for (int j = 0; j < life_height; j++) {
      int cnt = count_neighbour(i, j);
      if ((get_cell(i, j) == 1 && (cnt == 2 || cnt == 3)) ||
          (get_cell(i, j) == 0 && cnt == 3)) {
        //  Live cell with 2 or 3 neighbours continues living
        //  Dead cell with 3 neighbours becomes live
        set_cell_next(i, j, 1);
        if (get_cell(i, j) != 1) {
          draw_cell(i, j, 1);
          alive_count++;
        }
      } else {
        // All other cells die or remain dead
        set_cell_next(i, j, 0);
        if (get_cell(i, j) != 0) {
          draw_cell(i, j, 0);
          alive_count--;
        }
      }
    }
  }
  swap_boards();
}

// Prints all cells in board in their current state
void print_cells() {
  for (int i = 0; i < life_width; i++) {
    for (int j = 0; j < life_height; j++) {
      if (get_cell(i, j) == 1) {
        draw_cell(i, j, 1);
      } else {
        draw_cell(i, j, 0);
      }
    }
  }
}

int get_alive_count() { return alive_count; }