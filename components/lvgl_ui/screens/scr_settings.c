#include "scr_settings.h"
#include "screens.h"
#include "ui.h"
#include "esp_log.h"

static const char *TAG = "scr_settings";

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_settings_prepare(void)
{
    /* Dragging the slider (or the switch) horizontally must not change screen */
    lv_obj_clear_flag(ui_Slider1,   LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(ui_SwTimeout, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(ui_scrSettings, ui_screens_gesture_cb, LV_EVENT_GESTURE, NULL);
}

void scr_settings_init(void)
{
    ESP_LOGD(TAG, "init");
}

void scr_settings_uninit(void)
{
    ESP_LOGD(TAG, "uninit");
}

void scr_settings_step(void)
{
}
