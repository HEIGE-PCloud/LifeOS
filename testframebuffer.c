#include "mailbox.h"
#include "framebuffer.h"

unsigned int hsv_to_rgb(float h, float s, float v) {
    float r, g, b;
    int i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | (int)(b * 255);
}

void draw_rainbow_screen(int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float hue = (float)x / width;
            unsigned int color = hsv_to_rgb(hue, 1, 1);
            draw_raw_pixel(x, y, color);
        }
    }
}


int main(void) {
    init_framebuffer();
    draw_rainbow_screen(1920, 1080);
    return 0;
}