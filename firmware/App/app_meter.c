#include "app_meter.h"
#include "bsp_ina226.h"
#include "bsp_i2c.h"
#include "bsp_systick.h"
#include <string.h>
#include <stdio.h>

volatile meter_telemetry_t g_telemetry __attribute__((used)) = {
    .magic = 0x47443332,
    .v_min = 999.0f,
    .log = "System booting..."
};

static uint32_t s_last_calc_time = 0;

void app_meter_init(void)
{
    g_telemetry.ina_online = bsp_ina226_init() ? 1 : 0;
    s_last_calc_time = bsp_get_tick_ms();
}

void app_meter_reset_stats(void)
{
    g_telemetry.capacity_mah = 0.0f;
    g_telemetry.energy_mwh = 0.0f;
    g_telemetry.v_max = g_telemetry.voltage_v;
    g_telemetry.v_min = g_telemetry.voltage_v;
    g_telemetry.i_max = g_telemetry.current_a;
}

void app_meter_update(void)
{
    uint32_t now = bsp_get_tick_ms();
    float dt_hours = (float)(now - s_last_calc_time) / 3600000.0f; /* ms to hours */
    s_last_calc_time = now;

    g_telemetry.uptime_ms = now;
    g_telemetry.loop_count++;

    /* Probe INA226 if offline */
    if (!g_telemetry.ina_online) {
        if (bsp_i2c_check_device(0x40)) {
            g_telemetry.ina_online = bsp_ina226_init() ? 1 : 0;
        }
    }

    if (g_telemetry.ina_online) {
        ina226_data_t sensor;
        if (bsp_ina226_read_data(&sensor)) {
            g_telemetry.voltage_v = sensor.bus_voltage_v;
            g_telemetry.current_a = sensor.current_a;
            g_telemetry.power_w = sensor.power_w;
            g_telemetry.shunt_mv = sensor.shunt_voltage_mv;

            /* Accumulate energy & capacity when current > 1mA */
            if (sensor.current_a > 0.001f && dt_hours < 0.01f) {
                g_telemetry.capacity_mah += (sensor.current_a * 1000.0f) * dt_hours;
                g_telemetry.energy_mwh += (sensor.power_w * 1000.0f) * dt_hours;
            }

            /* Record extrema */
            if (sensor.bus_voltage_v > g_telemetry.v_max) {
                g_telemetry.v_max = sensor.bus_voltage_v;
            }
            if (sensor.bus_voltage_v < g_telemetry.v_min && sensor.bus_voltage_v > 0.5f) {
                g_telemetry.v_min = sensor.bus_voltage_v;
            }
            if (sensor.current_a > g_telemetry.i_max) {
                g_telemetry.i_max = sensor.current_a;
            }

            int v_int = (int)sensor.bus_voltage_v;
            int v_dec = (int)((sensor.bus_voltage_v - (float)v_int) * 1000.0f);
            int i_ma = (int)(sensor.current_a * 1000.0f);
            int p_mw = (int)(sensor.power_w * 1000.0f);

            snprintf((char *)g_telemetry.log, sizeof(g_telemetry.log),
                     "V:%d.%03dV I:%dmA P:%dmW",
                     v_int, v_dec >= 0 ? v_dec : -v_dec, i_ma, p_mw);
        } else {
            g_telemetry.ina_online = 0;
            snprintf((char *)g_telemetry.log, sizeof(g_telemetry.log), "INA226 Read Error");
        }
    } else {
        snprintf((char *)g_telemetry.log, sizeof(g_telemetry.log), "Waiting for INA226...");
    }
}
