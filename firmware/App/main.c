#include "gd32e23x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_systick.h"
#include "bsp_i2c.h"
#include "bsp_key.h"
#include "app_meter.h"
#include "app_ui.h"

/* Task Stack Sizes (in words) */
#define METER_TASK_STACK_SIZE   (128)   /* 128 words = 512 bytes */
#define UI_TASK_STACK_SIZE      (256)   /* 256 words = 1024 bytes */

/* Static Task Memory */
static StackType_t  s_meter_stack[METER_TASK_STACK_SIZE];
static StaticTask_t s_meter_tcb;

static StackType_t  s_ui_stack[UI_TASK_STACK_SIZE];
static StaticTask_t s_ui_tcb;

/* Meter Sampling & Telemetry Task (Priority: 3, Period: 100ms) */
static void vTaskMeter(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        bsp_i2c_lock();
        app_meter_update();
        bsp_i2c_unlock();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

/* User Interface & Button Task (Priority: 2, Period: 50ms) */
static void vTaskUI(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        /* Check button events */
        key_event_t evt = bsp_key_get_event();
        if (evt == KEY1_CLICK || evt == KEY2_CLICK) {
            app_ui_next_page();
        } else if (evt == KEY1_LONG_PRESS || evt == KEY2_LONG_PRESS) {
            app_meter_reset_stats();
        }

        /* Refresh OLED display */
        bsp_i2c_lock();
        app_ui_update();
        bsp_i2c_unlock();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

/* FreeRTOS Static Memory Hook for Idle Task */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE *pulIdleTaskStackSize)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* FreeRTOS Error Hooks */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    __asm volatile ("cpsid i" ::: "memory");
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    __asm volatile ("cpsid i" ::: "memory");
    for (;;);
}

int main(void)
{
    /* 1. Core clock & Systick basic init */
    bsp_systick_init();

    /* 2. Software I2C on PB6/PB7 (creates mutex) */
    bsp_i2c_init();

    /* 3. Keys on PA4 (KEY1) & PA3 (KEY2) */
    bsp_key_init();

    /* 4. Measurement & UI initial setup */
    app_meter_init();
    app_ui_init();

    /* 5. Create FreeRTOS tasks statically */
    xTaskCreateStatic(vTaskMeter,
                      "Meter",
                      METER_TASK_STACK_SIZE,
                      NULL,
                      3,
                      s_meter_stack,
                      &s_meter_tcb);

    xTaskCreateStatic(vTaskUI,
                      "UI",
                      UI_TASK_STACK_SIZE,
                      NULL,
                      2,
                      s_ui_stack,
                      &s_ui_tcb);

    /* 6. Start RTOS scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1) {
    }
}
