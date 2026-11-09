#include "bsp_i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static SemaphoreHandle_t s_i2c_mutex = NULL;
static StaticSemaphore_t s_i2c_mutex_buffer;

void bsp_i2c_lock(void)
{
    if (s_i2c_mutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xSemaphoreTake(s_i2c_mutex, portMAX_DELAY);
    }
}

void bsp_i2c_unlock(void)
{
    if (s_i2c_mutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xSemaphoreGive(s_i2c_mutex);
    }
}

static inline void i2c_delay(void)
{
    /* ~4-5us delay at 72MHz (approx 100kHz I2C clock) */
    volatile uint32_t i = 60;
    while (i--) {
        __NOP();
    }
}

static inline void scl_high(void)
{
    gpio_bit_set(I2C_PORT, I2C_SCL_PIN);
}

static inline void scl_low(void)
{
    gpio_bit_reset(I2C_PORT, I2C_SCL_PIN);
}

static inline void sda_high(void)
{
    gpio_bit_set(I2C_PORT, I2C_SDA_PIN);
}

static inline void sda_low(void)
{
    gpio_bit_reset(I2C_PORT, I2C_SDA_PIN);
}

static inline uint8_t sda_read(void)
{
    return (uint8_t)gpio_input_bit_get(I2C_PORT, I2C_SDA_PIN);
}

void bsp_i2c_init(void)
{
    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateMutexStatic(&s_i2c_mutex_buffer);
    }

    rcu_periph_clock_enable(I2C_RCU_PORT);

    /* Configure SCL (PB6) and SDA (PB7) as Open-Drain Output with internal Pull-up */
    gpio_mode_set(I2C_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C_SCL_PIN | I2C_SDA_PIN);
    gpio_output_options_set(I2C_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, I2C_SCL_PIN | I2C_SDA_PIN);

    /* Release bus: 9 clock pulses in case a slave is holding SDA low */
    sda_high();
    for (int k = 0; k < 9; k++) {
        scl_low();
        i2c_delay();
        scl_high();
        i2c_delay();
    }

    scl_high();
    sda_high();
    i2c_delay();
}

void bsp_i2c_start(void)
{
    sda_high();
    scl_high();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

void bsp_i2c_stop(void)
{
    sda_low();
    scl_low();
    i2c_delay();
    scl_high();
    i2c_delay();
    sda_high();
    i2c_delay();
}

bool bsp_i2c_write_byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) {
            sda_high();
        } else {
            sda_low();
        }
        byte <<= 1;
        i2c_delay();
        scl_high();
        i2c_delay();
        scl_low();
    }

    /* Read ACK */
    sda_high();
    i2c_delay();
    scl_high();
    i2c_delay();
    bool ack = (sda_read() == RESET);
    scl_low();
    i2c_delay();

    return ack;
}

uint8_t bsp_i2c_read_byte(bool ack)
{
    uint8_t byte = 0;
    sda_high();

    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        scl_high();
        i2c_delay();
        if (sda_read() != RESET) {
            byte |= 0x01;
        }
        scl_low();
        i2c_delay();
    }

    /* Send ACK or NACK */
    if (ack) {
        sda_low();
    } else {
        sda_high();
    }
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
    sda_high();
    i2c_delay();

    return byte;
}

bool bsp_i2c_check_device(uint8_t dev_addr)
{
    bsp_i2c_start();
    bool ack = bsp_i2c_write_byte((dev_addr << 1) | 0x00);
    bsp_i2c_stop();
    return ack;
}

uint8_t bsp_i2c_scan(uint8_t *found_addrs, uint8_t max_count)
{
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 128; addr++) {
        if (bsp_i2c_check_device(addr)) {
            if (found_addrs && count < max_count) {
                found_addrs[count] = addr;
            }
            count++;
        }
        i2c_delay();
    }
    return count;
}

bool bsp_i2c_write_reg16(uint8_t dev_addr, uint8_t reg, uint16_t val)
{
    bsp_i2c_start();
    if (!bsp_i2c_write_byte((dev_addr << 1) | 0x00)) {
        bsp_i2c_stop();
        return false;
    }
    if (!bsp_i2c_write_byte(reg)) {
        bsp_i2c_stop();
        return false;
    }
    if (!bsp_i2c_write_byte((uint8_t)(val >> 8))) {
        bsp_i2c_stop();
        return false;
    }
    if (!bsp_i2c_write_byte((uint8_t)(val & 0xFF))) {
        bsp_i2c_stop();
        return false;
    }
    bsp_i2c_stop();
    return true;
}

bool bsp_i2c_read_reg16(uint8_t dev_addr, uint8_t reg, uint16_t *val)
{
    bsp_i2c_start();
    if (!bsp_i2c_write_byte((dev_addr << 1) | 0x00)) {
        bsp_i2c_stop();
        return false;
    }
    if (!bsp_i2c_write_byte(reg)) {
        bsp_i2c_stop();
        return false;
    }

    bsp_i2c_start();
    if (!bsp_i2c_write_byte((dev_addr << 1) | 0x01)) {
        bsp_i2c_stop();
        return false;
    }

    uint8_t msb = bsp_i2c_read_byte(true);
    uint8_t lsb = bsp_i2c_read_byte(false);
    bsp_i2c_stop();

    if (val) {
        *val = ((uint16_t)msb << 8) | lsb;
    }
    return true;
}

bool bsp_i2c_write_bytes(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    bsp_i2c_start();
    if (!bsp_i2c_write_byte((dev_addr << 1) | 0x00)) {
        bsp_i2c_stop();
        return false;
    }
    for (uint16_t i = 0; i < len; i++) {
        if (!bsp_i2c_write_byte(data[i])) {
            bsp_i2c_stop();
            return false;
        }
    }
    bsp_i2c_stop();
    return true;
}
