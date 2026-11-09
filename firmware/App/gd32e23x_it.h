#ifndef GD32E23X_IT_H
#define GD32E23X_IT_H

#include "gd32e23x.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#endif /* GD32E23X_IT_H */
