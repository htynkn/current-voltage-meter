#include "bsp_i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define I2C_TIMEOUT_CYCLES   50000U

static SemaphoreHandle_t s_i2c_mutex = NULL;
static StaticSemaphore_t s_i2c_mutex_buffer;
static volatile TaskHandle_t s_dma_waiting_task = NULL;

static bool wait_flag_set(i2c_flag_enum flag)
{
    uint32_t to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, flag)) {
        if (--to == 0) return false;
    }
    return true;
}

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

void bsp_i2c_bus_reset(void)
{
    i2c_deinit(I2C0);

    /* GPIO mode for bus release: 9 clock pulses on SCL */
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_bit_set(GPIOB, GPIO_PIN_7);

    for (int i = 0; i < 9; i++) {
        gpio_bit_reset(GPIOB, GPIO_PIN_6);
        for (volatile int d = 0; d < 50; d++);
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        for (volatile int d = 0; d < 50; d++);
    }

    /* Switch back to I2C0 alternate function */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    /* Reinitialize I2C0 peripheral */
    i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    i2c_enable(I2C0);
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

void bsp_i2c_init(void)
{
    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateMutexStatic(&s_i2c_mutex_buffer);
    }

    /* Enable peripheral clocks */
    rcu_periph_clock_enable(RCU_DMA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C0);

    /* Configure PB6 (SCL) & PB7 (SDA) */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    /* Configure I2C0 at 400kHz Fast Mode */
    i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    i2c_enable(I2C0);
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    /* Enable DMA Channel1/2 IRQ in NVIC for I2C0_TX */
    nvic_irq_enable(DMA_Channel1_2_IRQn, 2);
}

bool bsp_i2c_check_device(uint8_t dev_addr)
{
    uint32_t to = I2C_TIMEOUT_CYCLES;
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
        if (--to == 0) {
            bsp_i2c_bus_reset();
            return false;
        }
    }

    i2c_start_on_bus(I2C0);
    if (!wait_flag_set(I2C_FLAG_SBSEND)) {
        bsp_i2c_bus_reset();
        return false;
    }

    i2c_master_addressing(I2C0, (dev_addr << 1), I2C_TRANSMITTER);
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND) && !i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
        if (--to == 0) {
            i2c_stop_on_bus(I2C0);
            return false;
        }
    }

    bool acked = false;
    if (i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        acked = true;
        i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    }
    if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
        i2c_flag_clear(I2C0, I2C_FLAG_AERR);
    }

    i2c_stop_on_bus(I2C0);
    to = I2C_TIMEOUT_CYCLES;
    while ((I2C_CTL0(I2C0) & I2C_CTL0_STOP) && --to);

    return acked;
}

uint8_t bsp_i2c_scan(uint8_t *found_addrs, uint8_t max_count)
{
    uint8_t count = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        if (bsp_i2c_check_device(addr)) {
            if (found_addrs && count < max_count) {
                found_addrs[count] = addr;
            }
            count++;
        }
    }
    return count;
}

bool bsp_i2c_write_bytes(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    if (len == 0) return true;

    uint32_t to = I2C_TIMEOUT_CYCLES;
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
        if (--to == 0) {
            bsp_i2c_bus_reset();
            return false;
        }
    }

    i2c_start_on_bus(I2C0);
    if (!wait_flag_set(I2C_FLAG_SBSEND)) {
        bsp_i2c_bus_reset();
        return false;
    }

    i2c_master_addressing(I2C0, (dev_addr << 1), I2C_TRANSMITTER);
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR) || --to == 0) {
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return false;
        }
    }
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    for (uint16_t i = 0; i < len; i++) {
        if (!wait_flag_set(I2C_FLAG_TBE)) {
            i2c_stop_on_bus(I2C0);
            return false;
        }
        i2c_data_transmit(I2C0, data[i]);
    }

    if (!wait_flag_set(I2C_FLAG_BTC)) {
        i2c_stop_on_bus(I2C0);
        return false;
    }

    i2c_stop_on_bus(I2C0);
    to = I2C_TIMEOUT_CYCLES;
    while ((I2C_CTL0(I2C0) & I2C_CTL0_STOP) && --to);

    return true;
}

bool bsp_i2c_write_reg16(uint8_t dev_addr, uint8_t reg, uint16_t val)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (uint8_t)(val >> 8);
    buf[2] = (uint8_t)(val & 0xFF);
    return bsp_i2c_write_bytes(dev_addr, buf, 3);
}

bool bsp_i2c_read_reg16(uint8_t dev_addr, uint8_t reg, uint16_t *val)
{
    uint32_t to = I2C_TIMEOUT_CYCLES;
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
        if (--to == 0) {
            bsp_i2c_bus_reset();
            return false;
        }
    }

    /* Start condition */
    i2c_start_on_bus(I2C0);
    if (!wait_flag_set(I2C_FLAG_SBSEND)) {
        bsp_i2c_bus_reset();
        return false;
    }

    /* Send address + W */
    i2c_master_addressing(I2C0, (dev_addr << 1), I2C_TRANSMITTER);
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR) || --to == 0) {
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return false;
        }
    }
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    /* Send register pointer */
    if (!wait_flag_set(I2C_FLAG_TBE)) {
        i2c_stop_on_bus(I2C0);
        return false;
    }
    i2c_data_transmit(I2C0, reg);
    if (!wait_flag_set(I2C_FLAG_BTC)) {
        i2c_stop_on_bus(I2C0);
        return false;
    }

    /* Repeated Start for 2-byte read */
    i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
    i2c_ack_config(I2C0, I2C_ACK_DISABLE);

    i2c_start_on_bus(I2C0);
    if (!wait_flag_set(I2C_FLAG_SBSEND)) {
        bsp_i2c_bus_reset();
        return false;
    }

    /* Send address + R */
    i2c_master_addressing(I2C0, (dev_addr << 1), I2C_RECEIVER);
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR) || --to == 0) {
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return false;
        }
    }
    /* Clear ADDSEND */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    /* Wait BTC (both 2 bytes received into shift reg + data reg) */
    if (!wait_flag_set(I2C_FLAG_BTC)) {
        i2c_stop_on_bus(I2C0);
        return false;
    }

    /* Send STOP before reading out */
    i2c_stop_on_bus(I2C0);

    /* Read MSB */
    uint8_t msb = i2c_data_receive(I2C0);
    /* Read LSB */
    uint8_t lsb = i2c_data_receive(I2C0);

    to = I2C_TIMEOUT_CYCLES;
    while ((I2C_CTL0(I2C0) & I2C_CTL0_STOP) && --to);

    /* Restore normal ACK state */
    i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    if (val) {
        *val = ((uint16_t)msb << 8) | lsb;
    }
    return true;
}

bool bsp_i2c_dma_transmit_async(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    if (len == 0) return true;

    uint32_t to = I2C_TIMEOUT_CYCLES;
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
        if (--to == 0) {
            bsp_i2c_bus_reset();
            return false;
        }
    }

    /* Configure DMA Channel 1 (I2C0_TX) */
    dma_channel_disable(DMA_CH1);
    dma_deinit(DMA_CH1);
    dma_parameter_struct dma_struct;
    dma_struct_para_init(&dma_struct);
    dma_struct.periph_addr  = (uint32_t)&I2C_DATA(I2C0);
    dma_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_addr  = (uint32_t)data;
    dma_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_struct.number       = len;
    dma_struct.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA_CH1, &dma_struct);

    dma_interrupt_flag_clear(DMA_CH1, DMA_INT_FLAG_FTF);
    dma_interrupt_enable(DMA_CH1, DMA_INT_FTF);

    /* Start condition */
    i2c_start_on_bus(I2C0);
    if (!wait_flag_set(I2C_FLAG_SBSEND)) {
        bsp_i2c_bus_reset();
        return false;
    }

    /* Send address + W */
    i2c_master_addressing(I2C0, (dev_addr << 1), I2C_TRANSMITTER);
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR) || --to == 0) {
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return false;
        }
    }
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    /* Record waiting task and start DMA transfer */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        s_dma_waiting_task = xTaskGetCurrentTaskHandle();
    }
    i2c_dma_config(I2C0, I2C_DMA_ON);
    dma_channel_enable(DMA_CH1);

    if (s_dma_waiting_task != NULL) {
        /* Task yields CPU and sleeps until DMA finish interrupt wakes it up */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
        s_dma_waiting_task = NULL;
    } else {
        /* If scheduler not started, poll DMA completion */
        while (!dma_flag_get(DMA_CH1, DMA_FLAG_FTF));
        dma_flag_clear(DMA_CH1, DMA_FLAG_FTF);
    }

    /* Wait for shift register to finish sending last byte */
    to = I2C_TIMEOUT_CYCLES;
    while (!i2c_flag_get(I2C0, I2C_FLAG_BTC) && --to);

    /* Send Stop condition */
    i2c_stop_on_bus(I2C0);
    to = I2C_TIMEOUT_CYCLES;
    while ((I2C_CTL0(I2C0) & I2C_CTL0_STOP) && --to);

    i2c_dma_config(I2C0, I2C_DMA_OFF);
    dma_channel_disable(DMA_CH1);

    return true;
}

/* DMA Channel 1 & 2 Interrupt Handler */
void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA_CH1, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA_CH1, DMA_INT_FLAG_FTF);
        dma_interrupt_disable(DMA_CH1, DMA_INT_FTF);

        if (s_dma_waiting_task != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(s_dma_waiting_task, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
