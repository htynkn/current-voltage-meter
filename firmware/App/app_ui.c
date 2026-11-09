#include "app_ui.h"
#include "app_meter.h"
#include "bsp_oled.h"
#include "bsp_i2c.h"
#include "bsp_systick.h"
#include <stdio.h>
#include <string.h>

static bool s_oled_ready = false;

static void fmt_float(char *buf, size_t len, float val, int decimals)
{
    if (val < 0.0f) {
        *buf++ = '-';
        val = -val;
        len--;
    }
    int int_part = (int)val;
    float frac_part = val - (float)int_part;

    int mult = 1;
    for (int i = 0; i < decimals; i++) mult *= 10;
    int frac_int = (int)(frac_part * mult + 0.5f);
    if (frac_int >= mult) {
        int_part++;
        frac_int -= mult;
    }

    if (decimals == 2) {
        snprintf(buf, len, "%d.%02d", int_part, frac_int);
    } else if (decimals == 3) {
        snprintf(buf, len, "%d.%03d", int_part, frac_int);
    } else {
        snprintf(buf, len, "%d.%d", int_part, frac_int);
    }
}

void app_ui_init(void)
{
    s_oled_ready = bsp_oled_init();
    g_telemetry.oled_online = s_oled_ready ? 1 : 0;
}

void app_ui_next_page(void)
{
    g_telemetry.ui_page = (g_telemetry.ui_page + 1) % 3;
}

static void draw_page_main(void)
{
    char num_buf[16];
    char line_buf[32];

    /* Top banner */
    bsp_oled_fill_rect(0, 0, 128, 11, OLED_COLOR_WHITE);
    bsp_oled_draw_str(16, 2, "POWER METER", OLED_FONT_6X8, OLED_COLOR_BLACK);
    bsp_oled_draw_str(104, 2, "P1", OLED_FONT_6X8, OLED_COLOR_BLACK);

    /* Large Voltage: 16x24 font */
    fmt_float(num_buf, sizeof(num_buf), g_telemetry.voltage_v, 2);
    snprintf(line_buf, sizeof(line_buf), "%sV", num_buf);
    bsp_oled_draw_large_str(8, 14, line_buf, OLED_COLOR_WHITE);

    /* Current in 8x16 font */
    fmt_float(num_buf, sizeof(num_buf), g_telemetry.current_a, 3);
    snprintf(line_buf, sizeof(line_buf), "I: %s A", num_buf);
    bsp_oled_draw_str(8, 38, line_buf, OLED_FONT_8X16, OLED_COLOR_WHITE);

    /* Bottom row: Power & Capacity */
    fmt_float(num_buf, sizeof(num_buf), g_telemetry.power_w, 2);
    snprintf(line_buf, sizeof(line_buf), "P:%sW", num_buf);
    bsp_oled_draw_str(4, 55, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    snprintf(line_buf, sizeof(line_buf), "%dmAh", (int)g_telemetry.capacity_mah);
    bsp_oled_draw_str(76, 55, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);
}

static void draw_page_stats(void)
{
    char num_buf[16];
    char line_buf[32];

    bsp_oled_fill_rect(0, 0, 128, 11, OLED_COLOR_WHITE);
    bsp_oled_draw_str(16, 2, "ENERGY & STATS", OLED_FONT_6X8, OLED_COLOR_BLACK);
    bsp_oled_draw_str(104, 2, "P2", OLED_FONT_6X8, OLED_COLOR_BLACK);

    fmt_float(num_buf, sizeof(num_buf), g_telemetry.capacity_mah, 1);
    snprintf(line_buf, sizeof(line_buf), "Cap: %s mAh", num_buf);
    bsp_oled_draw_str(4, 15, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    fmt_float(num_buf, sizeof(num_buf), g_telemetry.energy_mwh, 1);
    snprintf(line_buf, sizeof(line_buf), "Eng: %s mWh", num_buf);
    bsp_oled_draw_str(4, 26, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    fmt_float(num_buf, sizeof(num_buf), g_telemetry.v_max, 2);
    snprintf(line_buf, sizeof(line_buf), "Vmax: %sV", num_buf);
    bsp_oled_draw_str(4, 38, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    fmt_float(num_buf, sizeof(num_buf), g_telemetry.i_max, 3);
    snprintf(line_buf, sizeof(line_buf), "Imax: %sA", num_buf);
    bsp_oled_draw_str(68, 38, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    uint32_t sec = g_telemetry.uptime_ms / 1000;
    snprintf(line_buf, sizeof(line_buf), "Time: %02lu:%02lu:%02lu",
             (unsigned long)(sec / 3600),
             (unsigned long)((sec % 3600) / 60),
             (unsigned long)(sec % 60));
    bsp_oled_draw_str(4, 52, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);
}

static void draw_page_debug(void)
{
    char line_buf[32];

    bsp_oled_fill_rect(0, 0, 128, 11, OLED_COLOR_WHITE);
    bsp_oled_draw_str(16, 2, "HARDWARE INFO", OLED_FONT_6X8, OLED_COLOR_BLACK);
    bsp_oled_draw_str(104, 2, "P3", OLED_FONT_6X8, OLED_COLOR_BLACK);

    bsp_oled_draw_str(4, 16, "MCU: GD32E230C8T6", OLED_FONT_6X8, OLED_COLOR_WHITE);
    bsp_oled_draw_str(4, 27, "CLK: 72MHz (IRC8M)", OLED_FONT_6X8, OLED_COLOR_WHITE);

    snprintf(line_buf, sizeof(line_buf), "INA226: %s", g_telemetry.ina_online ? "ONLINE" : "OFFLINE");
    bsp_oled_draw_str(4, 38, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);

    snprintf(line_buf, sizeof(line_buf), "Shunt: 50mR (R2)");
    bsp_oled_draw_str(4, 49, line_buf, OLED_FONT_6X8, OLED_COLOR_WHITE);
}

void app_ui_update(void)
{
    /* If OLED is not yet ready, try auto-detecting and initializing */
    if (!s_oled_ready) {
        s_oled_ready = bsp_oled_init();
        g_telemetry.oled_online = s_oled_ready ? 1 : 0;
        if (!s_oled_ready) {
            return;
        }
    }

    bsp_oled_clear();

    switch (g_telemetry.ui_page) {
    case 0:
        draw_page_main();
        break;
    case 1:
        draw_page_stats();
        break;
    case 2:
        draw_page_debug();
        break;
    default:
        g_telemetry.ui_page = 0;
        draw_page_main();
        break;
    }

    bsp_oled_update();
}
