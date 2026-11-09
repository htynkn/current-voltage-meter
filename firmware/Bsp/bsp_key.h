#ifndef BSP_KEY_H
#define BSP_KEY_H

#include <stdint.h>
#include <stdbool.h>
#include "gd32e23x.h"

typedef enum {
    KEY_NONE = 0,
    KEY1_CLICK,
    KEY1_LONG_PRESS,
    KEY2_CLICK,
    KEY2_LONG_PRESS
} key_event_t;

void bsp_key_init(void);
key_event_t bsp_key_get_event(void);

#endif /* BSP_KEY_H */
