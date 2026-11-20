#include "ui_input.h"
#include "screens_update.h"
#include "bsp_button.h"
#include "lvgl.h"
#include "esp_log.h"

static const char *TAG = "ui_input";

#define UI_INPUT_MAX_PENDING 4      /* drop presses beyond this backlog */

/* Screens that receive the button. The splash is left out on purpose. */
static const ui_screens_t s_group_screens[] = { SCREEN_HOME, SCREEN_SYSTEM, SCREEN_SETTINGS };

static lv_group_t      *s_group   = NULL;
static lv_indev_drv_t   s_drv;
static lv_indev_t      *s_indev   = NULL;
/* Only touched from LVGL context: the update-queue handler and read_cb run
 * in the same task, so no locking is needed. */
static uint8_t          s_pending = 0;
static lv_indev_state_t s_last    = LV_INDEV_STATE_RELEASED;

/* ------------------------------------------------------------------ */

/** Runs in the esp_timer task of espressif/button: no lv_* calls allowed. */
static void on_button_click(void *btn, void *arg)
{
    (void)btn;
    (void)arg;
    ui_update_cmd(UI_UPDATE_BUTTON_PRESS, NULL);
}

/** Report each pending press as one PRESSED read followed by one RELEASED read. */
static void keypad_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->key = LV_KEY_ENTER;
    if (s_last == LV_INDEV_STATE_PRESSED) {
        s_last = LV_INDEV_STATE_RELEASED;
    } else if (s_pending > 0) {
        s_pending--;
        s_last = LV_INDEV_STATE_PRESSED;
    }
    data->state = s_last;
}

/* ------------------------------------------------------------------ */

void ui_input_push_enter(void)
{
    if (s_pending < UI_INPUT_MAX_PENDING) {
        s_pending++;
    }
}

void ui_input_focus_screen(ui_screens_t screen)
{
    if (!s_group) return;
    lv_obj_t *scr = ui_screens_get_obj(screen);
    if (scr && lv_obj_get_group(scr) == s_group) {
        lv_group_focus_obj(scr);
    }
}

void ui_input_init(void)
{
    s_group = lv_group_create();    /* not the default group on purpose */

    for (size_t i = 0; i < sizeof(s_group_screens) / sizeof(s_group_screens[0]); i++) {
        lv_obj_t *scr = ui_screens_get_obj(s_group_screens[i]);
        if (!scr) continue;
        lv_group_add_obj(s_group, scr);
        /* The theme draws a focus outline on keypad-focused objects */
        lv_obj_set_style_outline_width(scr, 0, LV_STATE_FOCUS_KEY);
    }

    lv_indev_drv_init(&s_drv);
    s_drv.type    = LV_INDEV_TYPE_KEYPAD;
    s_drv.read_cb = keypad_read_cb;
    s_indev = lv_indev_drv_register(&s_drv);
    lv_indev_set_group(s_indev, s_group);

    esp_err_t err = bsp_button_register_cb(BUTTON_SINGLE_CLICK, on_button_click, NULL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "button callback not registered: %s", esp_err_to_name(err));
    }
}
