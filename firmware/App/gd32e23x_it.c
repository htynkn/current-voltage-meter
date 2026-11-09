#include "gd32e23x_it.h"
#include "bsp_systick.h"

void NMI_Handler(void)
{
    while (1);
}

void HardFault_Handler(void)
{
    while (1);
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    bsp_systick_isr();
}
