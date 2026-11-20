#include "scr_system.h"
#include "screens.h"
#include "ui.h"
#include "esp_log.h"

static const char *TAG = "scr_system";

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_system_prepare(void)
{
    lv_obj_add_event_cb(ui_scrSystem, ui_screens_gesture_cb, LV_EVENT_GESTURE, NULL);
}

void scr_system_init(void)
{
    ESP_LOGD(TAG, "init");
}

void scr_system_uninit(void)
{
    ESP_LOGD(TAG, "uninit");
}

void scr_system_step(void)
{
}
