#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include "esp_err.h"
#include "iot_button.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_BUTTON_BOOT_GPIO    0

/** Create the BOOT button (GPIO0, active low). Call once at startup. */
esp_err_t bsp_button_init(void);

/**
 * Register a callback for a button event (e.g. BUTTON_SINGLE_CLICK).
 *
 * @warning The callback runs in the esp_timer task of espressif/button, NOT
 *          in LVGL context. It must not call any lv_* function; forward the
 *          event with ui_update_cmd() instead.
 */
esp_err_t bsp_button_register_cb(button_event_t event, button_cb_t cb, void *usr_data);

#ifdef __cplusplus
}
#endif

#endif
