#ifndef BSP_I2C_H
#define BSP_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "gd32e23x.h"

#define I2C_PORT        GPIOB
#define I2C_SCL_PIN     GPIO_PIN_6
#define I2C_SDA_PIN     GPIO_PIN_7
#define I2C_RCU_PORT    RCU_GPIOB

void bsp_i2c_init(void);
void bsp_i2c_start(void);
void bsp_i2c_stop(void);
bool bsp_i2c_write_byte(uint8_t byte);
uint8_t bsp_i2c_read_byte(bool ack);
bool bsp_i2c_check_device(uint8_t dev_addr);
uint8_t bsp_i2c_scan(uint8_t *found_addrs, uint8_t max_count);
bool bsp_i2c_write_reg16(uint8_t dev_addr, uint8_t reg, uint16_t val);
bool bsp_i2c_read_reg16(uint8_t dev_addr, uint8_t reg, uint16_t *val);
bool bsp_i2c_write_bytes(uint8_t dev_addr, const uint8_t *data, uint16_t len);

#endif /* BSP_I2C_H */
