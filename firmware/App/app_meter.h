#ifndef APP_METER_H
#define APP_METER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed)) {
    uint32_t magic;           /* 0x47443332 "GD32" */
    uint32_t uptime_ms;
    uint32_t loop_count;

    /* Device status */
    uint8_t  oled_online;     /* 1 = OLED SSD1306 (0x3C) online */
    uint8_t  ina_online;      /* 1 = INA226 (0x40) online */
    uint8_t  ui_page;         /* 0: Main Meter, 1: Stats & Energy, 2: Raw Debug */
    uint8_t  reserved;

    /* Electrical measurements */
    float    voltage_v;       /* Bus voltage in V */
    float    current_a;       /* Current in A */
    float    power_w;         /* Power in W */
    float    shunt_mv;        /* Shunt voltage in mV */

    /* Accumulated stats */
    float    capacity_mah;    /* Capacity in mAh */
    float    energy_mwh;      /* Energy in mWh */
    float    v_max;           /* Maximum voltage recorded */
    float    v_min;           /* Minimum voltage recorded */
    float    i_max;           /* Maximum current recorded */

    char     log[64];
} meter_telemetry_t;

extern volatile meter_telemetry_t g_telemetry;

void app_meter_init(void);
void app_meter_update(void);
void app_meter_reset_stats(void);

#endif /* APP_METER_H */
