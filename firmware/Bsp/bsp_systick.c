#include "bsp_systick.h"

static volatile uint32_t s_tick_count = 0;
static volatile uint32_t s_delay_count = 0;

void bsp_systick_init(void)
{
    /* 1ms tick based on SystemCoreClock (72MHz) */
    if (SysTick_Config(SystemCoreClock / 1000U)) {
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void bsp_delay_ms(uint32_t ms)
{
    uint32_t start = s_tick_count;
    while ((s_tick_count - start) < ms);
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
    return s_tick_count;
}

void bsp_systick_isr(void)
{
    s_tick_count++;
    if (s_delay_count > 0) {
        s_delay_count--;
    }
}
