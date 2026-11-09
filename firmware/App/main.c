#include "gd32e23x.h"
#include "bsp_systick.h"
#include "bsp_i2c.h"
#include "bsp_key.h"
#include "app_meter.h"
#include "app_ui.h"

int main(void)
{
    /* 1. System tick timer (72MHz based on internal IRC8M PLL) */
    bsp_systick_init();

    /* 2. Software I2C on PB6/PB7 */
    bsp_i2c_init();
    bsp_delay_ms(50);

    /* 3. Keys on PA4 (KEY1) & PA3 (KEY2) */
    bsp_key_init();

    /* 4. Measurement & UI */
    app_meter_init();
    app_ui_init();

    while (1) {
        /* Check button input */
        key_event_t evt = bsp_key_get_event();
        if (evt == KEY1_CLICK || evt == KEY2_CLICK) {
            app_ui_next_page();
        } else if (evt == KEY1_LONG_PRESS || evt == KEY2_LONG_PRESS) {
            app_meter_reset_stats();
        }

        /* Update telemetry (INA226) */
        app_meter_update();

        /* Update OLED display */
        app_ui_update();

        bsp_delay_ms(100);
    }
}
