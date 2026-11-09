#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* Chip & Architecture specific settings */
extern uint32_t SystemCoreClock;
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 100 ) /* 100 Hz = 10ms per tick */
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 64 )    /* 64 words = 256 bytes */
#define configMAX_TASK_NAME_LEN                 ( 8 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TIME_SLICING                  1
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/* Features configuration - minimal for 8KB RAM */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_QUEUE_SETS                    0
#define configUSE_TASK_NOTIFICATIONS            1

/* ARMv8-M / Cortex-M23 settings */
#define configENABLE_MPU                        0
#define configENABLE_FPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          0
#define configCHECK_HANDLER_INSTALLATION        0

/* Memory allocation */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) 1536 )

/* Hook functions */
#define configCHECK_FOR_STACK_OVERFLOW          1
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

/* Software timers disabled to save RAM */
#define configUSE_TIMERS                        0

/* API function inclusions */
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       0
#define INCLUDE_uxTaskGetStackHighWaterMark     1

/* Assert definition */
#define configASSERT( x )                       if( ( x ) == 0 ) { __asm volatile ( "cpsid i" ::: "memory" ); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
