#include "scr_settings.h"
#include "screens.h"
#include "ui.h"
#include "bsp_display.h"
#include "screens_timeout.h"
#include "esp_log.h"

static const char *TAG = "scr_settings";

/* Defaults restored by the physical button */
#define SETTINGS_DEFAULT_BRIGHTNESS  100
#define SETTINGS_DEFAULT_TIMEOUT_EN  false
#define SETTINGS_TIMEOUT_MS          30000
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

static void apply_timeout(bool enabled)
{
    disp_timeout_set_off_ms(enabled ? SETTINGS_TIMEOUT_MS : TIMEOUT_NEVER);
    ESP_LOGI(TAG, "display timeout %s", enabled ? "30 s" : "off");
}

/**
 * Apply both settings to the hardware AND the widgets. Used by the widget
 * events and by the button: lv_slider_set_value() and lv_obj_clear_state()
 * do not emit LV_EVENT_VALUE_CHANGED, so setting the widgets alone would
 * not reach the backlight or the timeout.
 */
static void apply_settings(int32_t brightness, bool timeout_en)
{
    lv_slider_set_value(ui_Slider1, brightness, LV_ANIM_ON);
    apply_brightness(brightness);

    if (timeout_en) {
        lv_obj_add_state(ui_SwTimeout, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_SwTimeout, LV_STATE_CHECKED);
    }
    apply_timeout(timeout_en);
}

/* ------------------------------------------------------------------ */
/* Event callbacks                                                     */
/* ------------------------------------------------------------------ */

static void on_brightness_changed(lv_event_t *e)
{
    (void)e;
    apply_brightness(lv_slider_get_value(ui_Slider1));    /* real time */
}

static void on_timeout_changed(lv_event_t *e)
{
    (void)e;
    apply_timeout(lv_obj_has_state(ui_SwTimeout, LV_STATE_CHECKED));
}

/** Physical button: restore defaults (100 % brightness, timeout off). */
static void on_key(lv_event_t *e)
{
    if (lv_event_get_key(e) != LV_KEY_ENTER) return;
    ESP_LOGI(TAG, "restoring defaults");
    apply_settings(SETTINGS_DEFAULT_BRIGHTNESS, SETTINGS_DEFAULT_TIMEOUT_EN);
}

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_settings_prepare(void)
{
    lv_slider_set_range(ui_Slider1, SETTINGS_MIN_BRIGHTNESS, SETTINGS_MAX_BRIGHTNESS);
    lv_slider_set_value(ui_Slider1, SETTINGS_DEFAULT_BRIGHTNESS, LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_Slider1, on_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);

    if (SETTINGS_DEFAULT_TIMEOUT_EN) {
        lv_obj_add_state(ui_SwTimeout, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_SwTimeout, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_SwTimeout, on_timeout_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_label_set_text_fmt(ui_LblBuild, "Compilado: %s %s", __DATE__, __TIME__);

    /* Dragging the slider (or the switch) horizontally must not change screen */
    lv_obj_clear_flag(ui_Slider1,   LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(ui_SwTimeout, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(ui_scrSettings, on_key, LV_EVENT_KEY, NULL);
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
