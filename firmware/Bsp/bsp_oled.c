#include "bsp_oled.h"
#include "bsp_oled_font.h"
#include "bsp_i2c.h"
#include "bsp_systick.h"
#include <string.h>
#include <stdlib.h>

static uint8_t s_oled_buffer[OLED_WIDTH * (OLED_HEIGHT / 8)];
static uint8_t s_oled_addr = OLED_I2C_ADDR;

static void oled_write_cmd(uint8_t cmd)
{
    bsp_i2c_start();
    bsp_i2c_write_byte(s_oled_addr << 1);
    bsp_i2c_write_byte(0x00); /* Co = 0, D/C# = 0 (command) */
    bsp_i2c_write_byte(cmd);
    bsp_i2c_stop();
}

bool bsp_oled_init(void)
{
    if (bsp_i2c_check_device(0x3C)) {
        s_oled_addr = 0x3C;
    } else if (bsp_i2c_check_device(0x3D)) {
        s_oled_addr = 0x3D;
    } else {
        return false;
    }

    bsp_delay_ms(20);

    /* Standard SSD1306 128x64 initialization sequence */
    oled_write_cmd(0xAE); /* display off */
    oled_write_cmd(0xD5); /* set display clock divide ratio/oscillator frequency */
    oled_write_cmd(0x80);
    oled_write_cmd(0xA8); /* set multiplex ratio(1 to 64) */
    oled_write_cmd(0x3F); /* 1/64 duty */
    oled_write_cmd(0xD3); /* set display offset */
    oled_write_cmd(0x00); /* not offset */
    oled_write_cmd(0x40); /* set start line address */
    oled_write_cmd(0x8D); /* set charge pump enable */
    oled_write_cmd(0x14);
    oled_write_cmd(0x20); /* set memory addressing mode */
    oled_write_cmd(0x02); /* page addressing mode */
    oled_write_cmd(0xA1); /* set segment re-map 0 to 127 */
    oled_write_cmd(0xC8); /* Set COM Output Scan Direction */
    oled_write_cmd(0xDA); /* set com pins hardware configuration */
    oled_write_cmd(0x12);
    oled_write_cmd(0x81); /* set contrast control register */
    oled_write_cmd(0xCF);
    oled_write_cmd(0xD9); /* set pre-charge period */
    oled_write_cmd(0xF1);
    oled_write_cmd(0xDB); /* set vcomh */
    oled_write_cmd(0x30);
    oled_write_cmd(0xA4); /* Output follows RAM content */
    oled_write_cmd(0xA6); /* set normal display */
    oled_write_cmd(0xAF); /* turn on SSD1306 panel */

    bsp_oled_clear();
    bsp_oled_update();

    return true;
}

void bsp_oled_clear(void)
{
    memset(s_oled_buffer, 0, sizeof(s_oled_buffer));
}

void bsp_oled_update(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page); /* Page start address */
        oled_write_cmd(0x00);        /* Lower column start address */
        oled_write_cmd(0x10);        /* Higher column start address */

        bsp_i2c_start();
        bsp_i2c_write_byte(s_oled_addr << 1);
        bsp_i2c_write_byte(0x40); /* Co = 0, D/C# = 1 (data) */
        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            bsp_i2c_write_byte(s_oled_buffer[page * OLED_WIDTH + col]);
        }
        bsp_i2c_stop();
    }
}

void bsp_oled_set_contrast(uint8_t contrast)
{
    oled_write_cmd(0x81);
    oled_write_cmd(contrast);
}

void bsp_oled_draw_pixel(int16_t x, int16_t y, oled_color_t color)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }

    uint16_t idx = (y / 8) * OLED_WIDTH + x;
    uint8_t bit = 1 << (y % 8);

    if (color == OLED_COLOR_WHITE) {
        s_oled_buffer[idx] |= bit;
    } else if (color == OLED_COLOR_BLACK) {
        s_oled_buffer[idx] &= ~bit;
    } else if (color == OLED_COLOR_INVERT) {
        s_oled_buffer[idx] ^= bit;
    }
}

void bsp_oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, oled_color_t color)
{
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        bsp_oled_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void bsp_oled_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, oled_color_t color)
{
    bsp_oled_draw_line(x, y, x + w - 1, y, color);
    bsp_oled_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
    bsp_oled_draw_line(x, y, x, y + h - 1, color);
    bsp_oled_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void bsp_oled_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, oled_color_t color)
{
    for (int16_t i = 0; i < h; i++) {
        bsp_oled_draw_line(x, y + i, x + w - 1, y + i, color);
    }
}

void bsp_oled_draw_char(int16_t x, int16_t y, char ch, oled_font_t font, oled_color_t color)
{
    if (ch < 32 || ch > 126) {
        ch = ' ';
    }
    uint8_t c_idx = ch - 32;

    if (font == OLED_FONT_6X8) {
        for (uint8_t row = 0; row < 8; row++) {
            uint8_t b = font_6x8[c_idx][row];
            for (uint8_t col = 0; col < 6; col++) {
                if (b & (0x80 >> col)) {
                    bsp_oled_draw_pixel(x + col, y + row, color);
                } else if (color == OLED_COLOR_WHITE) {
                    bsp_oled_draw_pixel(x + col, y + row, OLED_COLOR_BLACK);
                }
            }
        }
    } else {
        /* 8x16 font */
        for (uint8_t row = 0; row < 16; row++) {
            uint8_t b = font_8x16[c_idx][row];
            for (uint8_t col = 0; col < 8; col++) {
                if (b & (0x80 >> col)) {
                    bsp_oled_draw_pixel(x + col, y + row, color);
                } else if (color == OLED_COLOR_WHITE) {
                    bsp_oled_draw_pixel(x + col, y + row, OLED_COLOR_BLACK);
                }
            }
        }
    }
}

void bsp_oled_draw_str(int16_t x, int16_t y, const char *str, oled_font_t font, oled_color_t color)
{
    uint8_t char_w = (font == OLED_FONT_6X8) ? 6 : 8;
    while (*str) {
        bsp_oled_draw_char(x, y, *str, font, color);
        x += char_w;
        str++;
    }
}

static int get_large_char_index(char ch)
{
    const char *charset = "0123456789.- VAW";
    for (int i = 0; charset[i]; i++) {
        if (charset[i] == ch) return i;
    }
    return -1;
}

void bsp_oled_draw_large_digit(int16_t x, int16_t y, char ch, oled_color_t color)
{
    int idx = get_large_char_index(ch);
    if (idx < 0) return;

    /* 16x24: 24 rows, 2 bytes per row */
    for (uint8_t row = 0; row < 24; row++) {
        uint8_t b0 = font_digits_16x24[idx][row * 2];
        uint8_t b1 = font_digits_16x24[idx][row * 2 + 1];

        for (uint8_t col = 0; col < 8; col++) {
            if (b0 & (0x80 >> col)) {
                bsp_oled_draw_pixel(x + col, y + row, color);
            } else if (color == OLED_COLOR_WHITE) {
                bsp_oled_draw_pixel(x + col, y + row, OLED_COLOR_BLACK);
            }
        }
        for (uint8_t col = 0; col < 8; col++) {
            if (b1 & (0x80 >> col)) {
                bsp_oled_draw_pixel(x + 8 + col, y + row, color);
            } else if (color == OLED_COLOR_WHITE) {
                bsp_oled_draw_pixel(x + 8 + col, y + row, OLED_COLOR_BLACK);
            }
        }
    }
}

void bsp_oled_draw_large_str(int16_t x, int16_t y, const char *str, oled_color_t color)
{
    while (*str) {
        if (*str == '.') {
            bsp_oled_draw_large_digit(x, y, *str, color);
            x += 8; /* narrower for dot */
        } else {
            bsp_oled_draw_large_digit(x, y, *str, color);
            x += 16;
        }
        str++;
    }
}
