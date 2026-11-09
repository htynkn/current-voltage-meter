#include "gd32e23x_it.h"

void NMI_Handler(void)
{
    while (1);
}

void HardFault_Handler(void)
{
    while (1);
}

/*
 * SVC_Handler, PendSV_Handler, and SysTick_Handler
 * are implemented by FreeRTOS port in portasm.c and port.c
 */
