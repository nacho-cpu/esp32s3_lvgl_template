#ifndef __UI_INPUT_H__
#define __UI_INPUT_H__

#include "screens.h"

/**
 * Physical button as an LVGL input device.
 *
 *   [BOOT] -> espressif/button (esp_timer task)
 *          -> on_button_click() -> ui_update_cmd(UI_UPDATE_BUTTON_PRESS)   no lv_* here
 *          -> (LVGL context) queue handler -> ui_input_push_enter()
 *          -> keypad read_cb reports LV_KEY_ENTER pressed, then released
 *          -> LVGL sends LV_EVENT_KEY (LV_KEY_ENTER) to the focused object
 *          -> the focused object is the active screen, whose controller
 *             runs its context action.
 *
 * Screens react with:
 *     lv_obj_add_event_cb(ui_scrX, on_key, LV_EVENT_KEY, NULL);
 * The touch panel never generates LV_EVENT_KEY.
 *
 * Note: the keypad counts as user activity for LVGL, so a button press also
 * wakes the display from the inactivity timeout.
 */

/** Create the group and keypad indev, hook the button. Call after ui_screens_prepare(). */
void ui_input_init(void);

/** Queue one ENTER press for the keypad indev. LVGL context only. */
void ui_input_push_enter(void);

/** Focus a screen's object in the input group (no-op if it is not in the group). */
void ui_input_focus_screen(ui_screens_t screen);

#endif
