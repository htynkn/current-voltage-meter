#ifndef BSP_OLED_H
#define BSP_OLED_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_I2C_ADDR   0x3C

typedef enum {
    OLED_COLOR_BLACK = 0,
    OLED_COLOR_WHITE = 1,
    OLED_COLOR_INVERT = 2
} oled_color_t;

typedef enum {
    OLED_FONT_6X8  = 0,
    OLED_FONT_8X16 = 1
} oled_font_t;

bool bsp_oled_init(void);
void bsp_oled_clear(void);
void bsp_oled_update(void);
void bsp_oled_set_contrast(uint8_t contrast);
void bsp_oled_draw_pixel(int16_t x, int16_t y, oled_color_t color);
void bsp_oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, oled_color_t color);
void bsp_oled_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, oled_color_t color);
void bsp_oled_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, oled_color_t color);
void bsp_oled_draw_char(int16_t x, int16_t y, char ch, oled_font_t font, oled_color_t color);
void bsp_oled_draw_str(int16_t x, int16_t y, const char *str, oled_font_t font, oled_color_t color);
void bsp_oled_draw_large_digit(int16_t x, int16_t y, char ch, oled_color_t color);
void bsp_oled_draw_large_str(int16_t x, int16_t y, const char *str, oled_color_t color);

#endif /* BSP_OLED_H */
