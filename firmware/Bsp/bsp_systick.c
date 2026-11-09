#include "bsp_systick.h"
#include "FreeRTOS.h"
#include "task.h"

static volatile uint32_t s_tick_count = 0;

void bsp_systick_init(void)
{
    SystemCoreClockUpdate();
}

void bsp_delay_ms(uint32_t ms)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        TickType_t ticks = pdMS_TO_TICKS(ms);
        if (ticks == 0) {
            ticks = 1;
        }
        vTaskDelay(ticks);
    } else {
        while (ms--) {
            bsp_delay_us(1000);
        }
    }
}

void bsp_delay_us(uint32_t us)
{
    /* At 72MHz, each loop cycle is approx 3-4 instructions, so ~18-24 loops per microsecond */
    uint32_t count = us * (SystemCoreClock / 4000000U);
    while (count--) {
        __NOP();
    }
}

uint32_t bsp_get_tick_ms(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        return (uint32_t)(xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));
    }
    return s_tick_count;
}

void bsp_systick_isr(void)
{
    s_tick_count++;
}
