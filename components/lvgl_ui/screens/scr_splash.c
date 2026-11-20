#include "scr_splash.h"
#include "screens.h"
#include "ui.h"
#include "bsp_display.h"
#include "esp_log.h"

static const char *TAG = "scr_splash";

#define SPLASH_FADE_IN_MS   800
#define SPLASH_DOTS_STEP_MS 400   /* time per "." step */

/*
 * Startup note
 * ------------
 * ui_init() (SquareLine) loads ui_scrSplash *before* ui_screens_prepare()
 * registers our callbacks, so the LV_EVENT_SCREEN_LOAD_START/LOADED events of
 * the very first boot have already fired by the time prepare() runs. For that
 * reason the splash start-up (backlight fade-in + dot animation) is triggered
 * from scr_splash_init(), which the screen engine calls on its first tick
 * when it sees the splash as the active screen. Screen load/unload events are
 * still used for the splash -> home transition, which happens with all
 * callbacks already registered.
 */

static bool    s_navigating = false;
static int32_t s_dots_last  = -1;

/* ------------------------------------------------------------------ */
/* "Iniciando" -> "Iniciando..." animation                             */
/* ------------------------------------------------------------------ */

static void dots_exec_cb(void *var, int32_t v)
{
    static const char *txt[] = { "Iniciando", "Iniciando.", "Iniciando..", "Iniciando..." };

    int32_t n = v % 4;              /* 0..3 dots */
    if (n == s_dots_last) {
        return;                     /* avoid redrawing an unchanged label */
    }
    s_dots_last = n;
    lv_label_set_text_static((lv_obj_t *)var, txt[n]);
}

static void dots_start(void)
{
    lv_anim_del(ui_lblSplashStatus, dots_exec_cb);
    s_dots_last = -1;               /* force the first frame to draw */

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui_lblSplashStatus);
    lv_anim_set_exec_cb(&a, dots_exec_cb);
    lv_anim_set_values(&a, 0, 4);   /* 4 => last instant, 4 % 4 = 0 */
    lv_anim_set_time(&a, 4 * SPLASH_DOTS_STEP_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

/** Stop the animation and undo touch feedback. Safe to call more than once. */
static void splash_stop(void)
{
    lv_anim_del(ui_lblSplashStatus, dots_exec_cb);
    lv_obj_set_style_opa(ui_imgLogo, LV_OPA_COVER, 0);
}

/* ------------------------------------------------------------------ */
/* Event callbacks                                                     */
/* ------------------------------------------------------------------ */

static void on_pressed(lv_event_t *e)
{
    (void)e;
    lv_point_t p = { 0 };
    lv_indev_t *indev = lv_indev_get_act();
    if (indev) {
        lv_indev_get_point(indev, &p);
    }
    ESP_LOGI(TAG, "touch at (%d, %d)", (int)p.x, (int)p.y);

    /* Visual feedback: dim the logo while the finger is down */
    lv_obj_set_style_opa(ui_imgLogo, LV_OPA_70, 0);
}

static void on_released(lv_event_t *e)
{
    (void)e;
    lv_obj_set_style_opa(ui_imgLogo, LV_OPA_COVER, 0);
}

static void on_clicked(lv_event_t *e)
{
    (void)e;
    if (s_navigating) {
        return;                     /* guard against double navigation */
    }
    s_navigating = true;
    ESP_LOGI(TAG, "splash -> home");
    ui_screens_navigate(SCREEN_HOME);
}

static void on_unload_start(lv_event_t *e)
{
    (void)e;
    splash_stop();
}

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_splash_prepare(void)
{
    /* The whole screen is the touch target: children must not eat the touch */
    lv_obj_add_flag(ui_scrSplash, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_imgLogo, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_lblSplashStatus, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(ui_scrSplash, on_pressed,      LV_EVENT_PRESSED,             NULL);
    lv_obj_add_event_cb(ui_scrSplash, on_released,     LV_EVENT_RELEASED,            NULL);
    lv_obj_add_event_cb(ui_scrSplash, on_clicked,      LV_EVENT_CLICKED,             NULL);
    lv_obj_add_event_cb(ui_scrSplash, on_unload_start, LV_EVENT_SCREEN_UNLOAD_START, NULL);
}

void scr_splash_init(void)
{
    ESP_LOGD(TAG, "init: fade-in + dots");
    s_navigating = false;
    bsp_display_fade_in(SPLASH_FADE_IN_MS);
    dots_start();
}

void scr_splash_uninit(void)
{
    splash_stop();
}

void scr_splash_step(void)
{
}
