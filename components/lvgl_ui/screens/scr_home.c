#include <stdio.h>

#include "scr_home.h"
#include "screens.h"
#include "ui.h"
#include "bsp_battery.h"
#include "bsp_temp.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "scr_home";

#define HOME_REFRESH_MS 1000

/* Chip temperature thresholds for the status indicator */
#define HOME_TEMP_ALERT_C     50.0f   /* >= alert    */
#define HOME_TEMP_CRITICAL_C  65.0f   /* >= critical */

/*
 * Lifecycle
 * ---------
 * The home screen is driven by LVGL screen events instead of the engine's
 * init()/uninit() hooks:
 *   LV_EVENT_SCREEN_LOAD_START -> refresh_all()   (never show placeholder text)
 *   LV_EVENT_SCREEN_LOADED     -> create the 1 s refresh lv_timer
 *   LV_EVENT_SCREEN_UNLOADED   -> delete the timer
 * Unlike the splash, home is never the first screen, so these events always
 * fire after prepare() has registered the callbacks.
 */

static lv_timer_t *s_timer = NULL;

/* ------------------------------------------------------------------ */
/* Widgets                                                             */
/* ------------------------------------------------------------------ */

typedef enum {
    BAT_LEVEL_UNKNOWN = -1,
    BAT_LEVEL_LOW,      /* < 20 % or no reading */
    BAT_LEVEL_MID,      /* 20..50 %             */
    BAT_LEVEL_HIGH,     /* > 50 %               */
} bat_level_t;

static void battery_set_arc_color(int pct)
{
    static bat_level_t s_last_level = BAT_LEVEL_UNKNOWN;

    bat_level_t level;
    if (pct > 50) {
        level = BAT_LEVEL_HIGH;
    } else if (pct >= 20) {
        level = BAT_LEVEL_MID;
    } else {
        level = BAT_LEVEL_LOW;          /* includes pct < 0 (no reading) */
    }
    if (level == s_last_level) {
        return;                         /* only restyle when the band changes */
    }
    s_last_level = level;

    lv_palette_t palette = (level == BAT_LEVEL_HIGH) ? LV_PALETTE_GREEN
                         : (level == BAT_LEVEL_MID)  ? LV_PALETTE_AMBER
                                                     : LV_PALETTE_RED;
    lv_obj_set_style_arc_color(ui_ArcBattery, lv_palette_main(palette), LV_PART_INDICATOR);
}

static void refresh_battery(void)
{
    float v = -1.0f;
    int pct = bsp_battery_get_percent(&v);

    if (pct >= 0) {
        lv_arc_set_value(ui_ArcBattery, pct);
        lv_label_set_text_fmt(ui_LblBatpct, "%d%%", pct);
    } else {
        lv_arc_set_value(ui_ArcBattery, 0);
        lv_label_set_text_static(ui_LblBatpct, "--%");
    }
    battery_set_arc_color(pct);
}

/* ------------------------------------------------------------------ */
/* Status indicator                                                    */
/* ------------------------------------------------------------------ */

typedef enum {
    STATUS_UNKNOWN = -1,
    STATUS_NO_DATA,
    STATUS_NORMAL,
    STATUS_ALERT,
    STATUS_CRITICAL,
} home_status_t;

typedef struct {
    const char   *icon;
    const char   *text;
    lv_palette_t  palette;
} status_style_t;

static const status_style_t s_status_styles[] = {
    [STATUS_NO_DATA]  = { LV_SYMBOL_CLOSE,   "Sin dato", LV_PALETTE_GREY  },
    [STATUS_NORMAL]   = { LV_SYMBOL_OK,      "Normal",   LV_PALETTE_GREEN },
    [STATUS_ALERT]    = { LV_SYMBOL_WARNING, "Alerta",   LV_PALETTE_AMBER },
    [STATUS_CRITICAL] = { LV_SYMBOL_WARNING, "Critico",  LV_PALETTE_RED   },
};

static void status_set(bool valid, float celsius)
{
    static home_status_t s_last = STATUS_UNKNOWN;

    home_status_t st = !valid                          ? STATUS_NO_DATA
                     : (celsius >= HOME_TEMP_CRITICAL_C) ? STATUS_CRITICAL
                     : (celsius >= HOME_TEMP_ALERT_C)    ? STATUS_ALERT
                                                         : STATUS_NORMAL;
    if (st == s_last) {
        return;                         /* only restyle when the state changes */
    }
    s_last = st;

    const status_style_t *sty = &s_status_styles[st];
    lv_color_t color = lv_palette_main(sty->palette);
    lv_label_set_text_static(ui_LblStatusIcon, sty->icon);
    lv_label_set_text_static(ui_LblStatusText, sty->text);
    lv_obj_set_style_text_color(ui_LblStatusIcon, color, 0);
    lv_obj_set_style_border_color(ui_PnlStatus, color, 0);
}

static void refresh_temperature(void)
{
    float c = 0.0f;
    bool ok = (bsp_temp_get_celsius(&c) == ESP_OK);
    status_set(ok, c);
    if (ok) {
        /* LV_SPRINTF_USE_FLOAT is off, so format floats with libc snprintf */
        char buf[16];
        snprintf(buf, sizeof buf, "%.1f \xC2\xB0" "C", c);
        lv_label_set_text(ui_LblTemp, buf);
    } else {
        lv_label_set_text_static(ui_LblTemp, "--.- \xC2\xB0" "C");
    }
}

static void refresh_uptime(void)
{
    uint64_t s = esp_timer_get_time() / 1000000ULL;
    lv_label_set_text_fmt(ui_LblUpTime, "%02lu:%02lu:%02lu",
                          (unsigned long)(s / 3600),
                          (unsigned long)((s / 60) % 60),
                          (unsigned long)(s % 60));
}

static void refresh_all(void)
{
    refresh_battery();
    refresh_temperature();
    refresh_uptime();
}

/* ------------------------------------------------------------------ */
/* Timer and screen events                                             */
/* ------------------------------------------------------------------ */

static void home_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    refresh_all();
}

/** Physical button: refresh now and restart the 1 s period. */
static void on_key(lv_event_t *e)
{
    if (lv_event_get_key(e) != LV_KEY_ENTER) return;
    refresh_all();
    if (s_timer) {
        lv_timer_reset(s_timer);
    }
    ui_screens_flash(ui_PnlStatus);
}

static void on_load_start(lv_event_t *e)
{
    (void)e;
    refresh_all();
}

static void on_loaded(lv_event_t *e)
{
    (void)e;
    if (s_timer == NULL) {
        s_timer = lv_timer_create(home_timer_cb, HOME_REFRESH_MS, NULL);
        ESP_LOGD(TAG, "refresh timer started");
    }
}

static void on_unloaded(lv_event_t *e)
{
    (void)e;
    if (s_timer != NULL) {
        lv_timer_del(s_timer);
        s_timer = NULL;
        ESP_LOGD(TAG, "refresh timer stopped");
    }
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

    /* Right column: temperature and uptime above the status panel.
     * The SquareLine export places LblTemp over PnlStatus, so fix it here. */
    lv_obj_align(ui_LblTemp,   LV_ALIGN_CENTER, 80, -70);
    lv_obj_align(ui_LblUpTime, LV_ALIGN_CENTER, 80, -35);

    /* Status panel: icon + text in a row (layout not set in the export) */
    lv_obj_set_flex_flow(ui_PnlStatus, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_PnlStatus, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(ui_PnlStatus, 4, 0);
    lv_obj_set_style_pad_column(ui_PnlStatus, 8, 0);
    lv_obj_set_style_border_width(ui_PnlStatus, 2, 0);
    lv_obj_clear_flag(ui_PnlStatus, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_scrHome, on_key,        LV_EVENT_KEY,               NULL);
    lv_obj_add_event_cb(ui_scrHome, on_load_start, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(ui_scrHome, on_loaded,     LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(ui_scrHome, on_unloaded,   LV_EVENT_SCREEN_UNLOADED,   NULL);
    lv_obj_add_event_cb(ui_scrHome, ui_screens_gesture_cb, LV_EVENT_GESTURE,   NULL);
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
