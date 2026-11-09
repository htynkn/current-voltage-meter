#ifndef BSP_SYSTICK_H
#define BSP_SYSTICK_H

#include <stdint.h>
#include "gd32e23x.h"

void bsp_systick_init(void);
void bsp_delay_ms(uint32_t ms);
void bsp_delay_us(uint32_t us);
uint32_t bsp_get_tick_ms(void);
void bsp_systick_isr(void);

#endif /* BSP_SYSTICK_H */
