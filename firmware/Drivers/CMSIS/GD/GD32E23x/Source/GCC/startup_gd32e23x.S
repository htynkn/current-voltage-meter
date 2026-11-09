/**
  ******************************************************************************
  * @file      startup_gd32e23x.S
  * @author    Auto-generated for GD32E230 with GCC
  * @brief     GD32E23x Devices vector table for GCC toolchain.
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m23
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* Linker script symbols */
.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   r0, =_estack
  mov   sp, r0

  /* Copy data segment initializers from flash to SRAM */
  ldr   r0, =_sdata
  ldr   r1, =_edata
  ldr   r2, =_sidata
  movs  r3, #0
  b     LoopCopyDataInit

CopyDataInit:
  ldr   r4, [r2, r3]
  str   r4, [r0, r3]
  adds  r3, r3, #4

LoopCopyDataInit:
  adds  r4, r0, r3
  cmp   r4, r1
  bcc   CopyDataInit

  /* Zero fill the bss segment */
  ldr   r2, =_sbss
  ldr   r4, =_ebss
  movs  r3, #0
  b     LoopFillZerobss

FillZerobss:
  str   r3, [r2]
  adds  r2, r2, #4

LoopFillZerobss:
  cmp   r2, r4
  bcc   FillZerobss

  /* Call SystemInit */
  bl    SystemInit

  /* Call static constructors if any */
  bl    __libc_init_array

  /* Call main */
  bl    main

LoopForever:
  b     LoopForever

  .size Reset_Handler, .-Reset_Handler

  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word 0
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler

  /* External Interrupts */
  .word WWDGT_IRQHandler
  .word LVD_IRQHandler
  .word RTC_IRQHandler
  .word FMC_IRQHandler
  .word RCU_IRQHandler
  .word EXTI0_1_IRQHandler
  .word EXTI2_3_IRQHandler
  .word EXTI4_15_IRQHandler
  .word 0
  .word DMA_Channel0_IRQHandler
  .word DMA_Channel1_2_IRQHandler
  .word DMA_Channel3_4_IRQHandler
  .word ADC_CMP_IRQHandler
  .word TIMER0_BRK_UP_TRG_COM_IRQHandler
  .word TIMER0_Channel_IRQHandler
  .word 0
  .word TIMER2_IRQHandler
  .word TIMER5_IRQHandler
  .word 0
  .word TIMER13_IRQHandler
  .word TIMER14_IRQHandler
  .word TIMER15_IRQHandler
  .word TIMER16_IRQHandler
  .word I2C0_EV_IRQHandler
  .word I2C1_EV_IRQHandler
  .word SPI0_IRQHandler
  .word SPI1_IRQHandler
  .word USART0_IRQHandler
  .word USART1_IRQHandler
  .word 0
  .word 0
  .word 0
  .word I2C0_ER_IRQHandler
  .word 0
  .word I2C1_ER_IRQHandler

  /* Weak aliases for Exception and IRQ Handlers */
  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler

  .weak WWDGT_IRQHandler
  .thumb_set WWDGT_IRQHandler,Default_Handler

  .weak LVD_IRQHandler
  .thumb_set LVD_IRQHandler,Default_Handler

  .weak RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_Handler

  .weak FMC_IRQHandler
  .thumb_set FMC_IRQHandler,Default_Handler

  .weak RCU_IRQHandler
  .thumb_set RCU_IRQHandler,Default_Handler

  .weak EXTI0_1_IRQHandler
  .thumb_set EXTI0_1_IRQHandler,Default_Handler

  .weak EXTI2_3_IRQHandler
  .thumb_set EXTI2_3_IRQHandler,Default_Handler

  .weak EXTI4_15_IRQHandler
  .thumb_set EXTI4_15_IRQHandler,Default_Handler

  .weak DMA_Channel0_IRQHandler
  .thumb_set DMA_Channel0_IRQHandler,Default_Handler

  .weak DMA_Channel1_2_IRQHandler
  .thumb_set DMA_Channel1_2_IRQHandler,Default_Handler

  .weak DMA_Channel3_4_IRQHandler
  .thumb_set DMA_Channel3_4_IRQHandler,Default_Handler

  .weak ADC_CMP_IRQHandler
  .thumb_set ADC_CMP_IRQHandler,Default_Handler

  .weak TIMER0_BRK_UP_TRG_COM_IRQHandler
  .thumb_set TIMER0_BRK_UP_TRG_COM_IRQHandler,Default_Handler

  .weak TIMER0_Channel_IRQHandler
  .thumb_set TIMER0_Channel_IRQHandler,Default_Handler

  .weak TIMER2_IRQHandler
  .thumb_set TIMER2_IRQHandler,Default_Handler

  .weak TIMER5_IRQHandler
  .thumb_set TIMER5_IRQHandler,Default_Handler

  .weak TIMER13_IRQHandler
  .thumb_set TIMER13_IRQHandler,Default_Handler

  .weak TIMER14_IRQHandler
  .thumb_set TIMER14_IRQHandler,Default_Handler

  .weak TIMER15_IRQHandler
  .thumb_set TIMER15_IRQHandler,Default_Handler

  .weak TIMER16_IRQHandler
  .thumb_set TIMER16_IRQHandler,Default_Handler

  .weak I2C0_EV_IRQHandler
  .thumb_set I2C0_EV_IRQHandler,Default_Handler

  .weak I2C1_EV_IRQHandler
  .thumb_set I2C1_EV_IRQHandler,Default_Handler

  .weak SPI0_IRQHandler
  .thumb_set SPI0_IRQHandler,Default_Handler

  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler

  .weak USART0_IRQHandler
  .thumb_set USART0_IRQHandler,Default_Handler

  .weak USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_Handler

  .weak I2C0_ER_IRQHandler
  .thumb_set I2C0_ER_IRQHandler,Default_Handler

  .weak I2C1_ER_IRQHandler
  .thumb_set I2C1_ER_IRQHandler,Default_Handler
