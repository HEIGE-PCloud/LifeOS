#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
typedef enum {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  WHITE,
  GRAY,
  LIGHT_BLUE,
  LIGHT_GREEN,
  LIGHT_CYAN,
  LIGHT_RED,
  LIGHT_MAGENTA,
  YELLOW,
  BRIGHT_WHITE,
} color_t;

enum {
  FONT_WIDTH = 8,
  FONT_HEIGHT = 8,
  FONT_BPG = 8,  // Bytes per glyph
  FONT_BPL = 1,  // Bytes per line
  FONT_NUMGLYPHS = 224
};void init_framebuffer();
void draw_raw_pixel(int x, int y, unsigned int val);
void draw_pixel(int x, int y, color_t color);
void draw_char(unsigned char ch, int x, int y, color_t color);
void draw_string(int x, int y, char *s, color_t color);
void draw_rect(int x1, int y1, int x2, int y2, color_t color, int fill);
void draw_circle(int x0, int y0, int radius, color_t color, int fill);
void draw_line(int x1, int y1, int x2, int y2, color_t color);
void clear_screen();
void draw_number(int num, int x, int y, int attr);
#endif
