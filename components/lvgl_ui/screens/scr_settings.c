#include "scr_settings.h"
#include "screens.h"
#include "ui.h"
#include "bsp_display.h"
#include "esp_log.h"

static const char *TAG = "scr_settings";

/* Defaults restored by the physical button */
#define SETTINGS_DEFAULT_BRIGHTNESS  100
#define SETTINGS_MIN_BRIGHTNESS      10     /* slider never turns the panel black */
#define SETTINGS_MAX_BRIGHTNESS      100

/*
 * Widget names come from the SquareLine export: the brightness slider is
 * ui_Slider1 and the percentage label is ui_LblBrightnessPct_.
 */

/* ------------------------------------------------------------------ */
/* Settings                                                            */
/* ------------------------------------------------------------------ */

static void apply_brightness(int32_t pct)
{
    bsp_display_set_brightness((uint8_t)pct);
    lv_label_set_text_fmt(ui_LblBrightnessPct_, "%d%%", (int)pct);
}

/* ------------------------------------------------------------------ */
/* Event callbacks                                                     */
/* ------------------------------------------------------------------ */

static void on_brightness_changed(lv_event_t *e)
{
    (void)e;
    apply_brightness(lv_slider_get_value(ui_Slider1));    /* real time */
}

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_settings_prepare(void)
{
    lv_slider_set_range(ui_Slider1, SETTINGS_MIN_BRIGHTNESS, SETTINGS_MAX_BRIGHTNESS);
    lv_slider_set_value(ui_Slider1, SETTINGS_DEFAULT_BRIGHTNESS, LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_Slider1, on_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_label_set_text_fmt(ui_LblBuild, "Compilado: %s %s", __DATE__, __TIME__);

    /* Dragging the slider (or the switch) horizontally must not change screen */
    lv_obj_clear_flag(ui_Slider1,   LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(ui_SwTimeout, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(ui_scrSettings, ui_screens_gesture_cb, LV_EVENT_GESTURE, NULL);
}

void scr_settings_init(void)
{
    ESP_LOGD(TAG, "init");
    /* Sync with the real backlight (100 % after the splash fade-in) */
    int32_t pct = bsp_display_get_brightness();
    if (pct < SETTINGS_MIN_BRIGHTNESS) {
        pct = SETTINGS_MIN_BRIGHTNESS;
    }
    lv_slider_set_value(ui_Slider1, pct, LV_ANIM_OFF);
    lv_label_set_text_fmt(ui_LblBrightnessPct_, "%d%%", (int)pct);
}

void scr_settings_uninit(void)
{
    ESP_LOGD(TAG, "uninit");
}

void scr_settings_step(void)
{
}
