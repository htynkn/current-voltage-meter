#ifndef DEBUG_INFO_H
#define DEBUG_INFO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed)) {
    uint32_t magic;           /* 0x47443332: 'G''D''3''2' */
    uint32_t loop_count;
    uint32_t uptime_ms;

    /* I2C scan */
    uint8_t  i2c_found_count;
    uint8_t  i2c_addrs[8];

    /* INA226 details */
    uint8_t  ina_detected;
    uint16_t ina_manuf_id;    /* expected 0x5449 */
    uint16_t ina_die_id;      /* expected 0x2260 */
    uint16_t ina_config;
    uint16_t ina_raw_bus;
    int16_t  ina_raw_shunt;
    int16_t  ina_raw_current;

    float    voltage_v;
    float    current_ma;
    float    power_mw;
    float    shunt_mv;

    char     status_msg[64];
} debug_info_t;

extern volatile debug_info_t g_debug;

#endif /* DEBUG_INFO_H */
