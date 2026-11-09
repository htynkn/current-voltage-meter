#include "bsp_key.h"
#include "bsp_systick.h"

/* KEY1: PA4, KEY2: PA3 (Active LOW) */
#define KEY_PORT        GPIOA
#define KEY1_PIN        GPIO_PIN_4
#define KEY2_PIN        GPIO_PIN_3
#define KEY_RCU_PORT    RCU_GPIOA

void bsp_key_init(void)
{
    rcu_periph_clock_enable(KEY_RCU_PORT);
    gpio_mode_set(KEY_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, KEY1_PIN | KEY2_PIN);
}

key_event_t bsp_key_get_event(void)
{
    static uint32_t k1_press_time = 0;
    static uint32_t k2_press_time = 0;
    static bool k1_pressed = false;
    static bool k2_pressed = false;

    uint32_t now = bsp_get_tick_ms();
    key_event_t event = KEY_NONE;

    /* KEY1 (PA4) */
    if (gpio_input_bit_get(KEY_PORT, KEY1_PIN) == RESET) {
        if (!k1_pressed) {
            k1_pressed = true;
            k1_press_time = now;
        } else {
            if ((now - k1_press_time) > 800 && k1_press_time != 0) {
                event = KEY1_LONG_PRESS;
                k1_press_time = 0; /* fire once */
            }
        }
    } else {
        if (k1_pressed) {
            if (k1_press_time != 0 && (now - k1_press_time) > 30) {
                event = KEY1_CLICK;
            }
            k1_pressed = false;
            k1_press_time = 0;
        }
    }

    if (event != KEY_NONE) return event;

    /* KEY2 (PA3) */
    if (gpio_input_bit_get(KEY_PORT, KEY2_PIN) == RESET) {
        if (!k2_pressed) {
            k2_pressed = true;
            k2_press_time = now;
        } else {
            if ((now - k2_press_time) > 800 && k2_press_time != 0) {
                event = KEY2_LONG_PRESS;
                k2_press_time = 0; /* fire once */
            }
        }
    } else {
        if (k2_pressed) {
            if (k2_press_time != 0 && (now - k2_press_time) > 30) {
                event = KEY2_CLICK;
            }
            k2_pressed = false;
            k2_press_time = 0;
        }
    }

    return event;
}
