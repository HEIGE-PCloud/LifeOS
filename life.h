#ifndef LIFE_H
#define LIFE_H
typedef struct {
    int x;
    int y;
} pos_t;
typedef struct {
    pos_t *pos;
    int len;
    int max_ticks;
    char *name;
} start_state_t;
void init_life(int w, int h, int s);
void init_cells(start_state_t *state);
void set_cell(int x, int y, char value);
char get_cell(int x, int y);
void next_tick();
int count_neighbour(int x, int y);
void print_cells();
void clear_life();
int get_alive_count();
#endif
