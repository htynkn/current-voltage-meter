#ifndef BSP_INA226_H
#define BSP_INA226_H

#include <stdint.h>
#include <stdbool.h>

#define INA226_I2C_ADDR         0x40

#define INA226_REG_CONFIG       0x00
#define INA226_REG_SHUNTVOLT    0x01
#define INA226_REG_BUSVOLT      0x02
#define INA226_REG_POWER        0x03
#define INA226_REG_CURRENT      0x04
#define INA226_REG_CALIB        0x05
#define INA226_REG_MASK_ENABLE  0x06
#define INA226_REG_ALERT_LIMIT  0x07
#define INA226_REG_MANUF_ID     0xFE
#define INA226_REG_DIE_ID       0xFF

typedef struct {
    float bus_voltage_v;   /* Bus voltage in Volts */
    float current_a;       /* Current in Amperes */
    float power_w;         /* Power in Watts */
    float shunt_voltage_mv;/* Shunt voltage in mV */
    bool  is_connected;    /* Sensor online flag */
} ina226_data_t;

bool bsp_ina226_init(void);
bool bsp_ina226_read_data(ina226_data_t *data);

#endif /* BSP_INA226_H */
