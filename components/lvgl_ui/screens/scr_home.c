#include "scr_home.h"
#include "screens.h"
#include "ui.h"
#include "bsp_battery.h"
#include "esp_log.h"

static const char *TAG = "scr_home";

/*
 * Lifecycle
 * ---------
 * The home screen is driven by LVGL screen events instead of the engine's
 * init()/uninit() hooks:
 *   LV_EVENT_SCREEN_LOAD_START -> refresh_all()   (never show placeholder text)
 * Unlike the splash, home is never the first screen, so these events always
 * fire after prepare() has registered the callbacks.
 */

/* ------------------------------------------------------------------ */
/* Widgets                                                             */
/* ------------------------------------------------------------------ */

static void refresh_battery(void)
{
    float v = -1.0f;
    int pct = bsp_battery_get_percent(&v);

    if (pct >= 0) {
        lv_arc_set_value(ui_ArcBattery, pct);
        lv_label_set_text_fmt(ui_LblBatPct, "%d%%", pct);
    } else {
        lv_arc_set_value(ui_ArcBattery, 0);
        lv_label_set_text_static(ui_LblBatPct, "--%");
    }
}

static void refresh_all(void)
{
    refresh_battery();
}

/* ------------------------------------------------------------------ */
/* Screen events                                                     */
/* ------------------------------------------------------------------ */

static void on_load_start(lv_event_t *e)
{
    (void)e;
    refresh_all();
}

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_home_prepare(void)
{
    /* Battery arc: 270 deg custom-angle gauge, open at the bottom.
     * Set here so it does not depend on the SquareLine settings. */
    lv_arc_set_range(ui_ArcBattery, 0, 100);
    lv_arc_set_bg_angles(ui_ArcBattery, 135, 45);
    lv_arc_set_rotation(ui_ArcBattery, 0);
    lv_arc_set_mode(ui_ArcBattery, LV_ARC_MODE_NORMAL);
    lv_arc_set_value(ui_ArcBattery, 0);
    lv_obj_remove_style(ui_ArcBattery, NULL, LV_PART_KNOB);   /* no knob */
    lv_obj_clear_flag(ui_ArcBattery, LV_OBJ_FLAG_CLICKABLE);  /* indicator only */

    lv_obj_add_event_cb(ui_scrHome, on_load_start, LV_EVENT_SCREEN_LOAD_START, NULL);
}

void scr_home_init(void)
{
    ESP_LOGD(TAG, "init");
}

void scr_home_uninit(void)
{
    ESP_LOGD(TAG, "uninit");
}

void scr_home_step(void)
{
}
