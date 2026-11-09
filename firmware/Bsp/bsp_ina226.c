#include "bsp_ina226.h"
#include "bsp_i2c.h"
#include "bsp_systick.h"

/*
 * Shunt Resistor: R2 = 50 mOhm = 0.050 Ohm
 * With Current_LSB = 0.1 mA (100 uA):
 *   CAL = 0.00512 / (Current_LSB * R_shunt)
 *       = 0.00512 / (0.0001 * 0.05) = 1024 (0x0400)
 */
#define INA226_CAL_VAL          1024
#define INA226_CURRENT_LSB_A    0.0001f /* 100uA = 0.1mA */
#define INA226_POWER_LSB_W      0.0025f /* 25 * Current_LSB = 2.5mW */

bool bsp_ina226_init(void)
{
    uint16_t id = 0;
    
    /* Check Manufacturer ID: expected 0x5449 ("TI") */
    if (!bsp_i2c_read_reg16(INA226_I2C_ADDR, INA226_REG_MANUF_ID, &id) || id != 0x5449) {
        return false;
    }

    /*
     * Config Register (0x00):
     * Bits[11:9] AVG = 010 (16 averages)
     * Bits[8:6]  VBUSCT = 100 (1.1 ms)
     * Bits[5:3]  VSHCT  = 100 (1.1 ms)
     * Bits[2:0]  MODE   = 111 (Continuous shunt and bus)
     * Value: 0x4527
     */
    if (!bsp_i2c_write_reg16(INA226_I2C_ADDR, INA226_REG_CONFIG, 0x4527)) {
        return false;
    }

    /* Write calibration register */
    if (!bsp_i2c_write_reg16(INA226_I2C_ADDR, INA226_REG_CALIB, INA226_CAL_VAL)) {
        return false;
    }

    return true;
}

bool bsp_ina226_read_data(ina226_data_t *data)
{
    if (!data) return false;

    uint16_t raw_bus = 0;
    int16_t raw_shunt = 0;
    int16_t raw_current = 0;
    uint16_t raw_power = 0;

    /* Read Bus Voltage: LSB = 1.25 mV */
    if (!bsp_i2c_read_reg16(INA226_I2C_ADDR, INA226_REG_BUSVOLT, &raw_bus)) {
        data->is_connected = false;
        return false;
    }

    /* Read Shunt Voltage: LSB = 2.5 uV */
    if (!bsp_i2c_read_reg16(INA226_I2C_ADDR, INA226_REG_SHUNTVOLT, (uint16_t *)&raw_shunt)) {
        data->is_connected = false;
        return false;
    }

    /* Read Current: LSB = 0.1 mA */
    bsp_i2c_read_reg16(INA226_I2C_ADDR, INA226_REG_CURRENT, (uint16_t *)&raw_current);

    /* Read Power: LSB = 2.5 mW */
    bsp_i2c_read_reg16(INA226_I2C_ADDR, INA226_REG_POWER, &raw_power);

    data->bus_voltage_v = (float)raw_bus * 0.00125f;
    data->shunt_voltage_mv = (float)raw_shunt * 0.0025f;

    /* 
     * If calibration register is active, use current register;
     * Fallback to shunt calculation: I = Vshunt / 0.050 Ohm
     */
    if (raw_current != 0) {
        data->current_a = (float)raw_current * INA226_CURRENT_LSB_A;
    } else {
        data->current_a = (data->shunt_voltage_mv / 50.0f); /* I(A) = Vshunt(mV)/50(mOhm) */
    }

    /* Filter minor noise around 0mA */
    if (data->current_a > -0.0005f && data->current_a < 0.0005f) {
        data->current_a = 0.0f;
    }

    data->power_w = data->bus_voltage_v * data->current_a;
    if (data->power_w < 0.0f) {
        data->power_w = -data->power_w;
    }

    data->is_connected = true;
    return true;
}
