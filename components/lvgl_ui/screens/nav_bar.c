#include <stdint.h>

#include "nav_bar.h"
#include "lvgl.h"

typedef struct {
    ui_screens_t screen;
    const char  *symbol;
} nav_item_t;

static const nav_item_t s_items[] = {
    { SCREEN_HOME,     LV_SYMBOL_HOME     },
    { SCREEN_SYSTEM,   LV_SYMBOL_LIST     },
    { SCREEN_SETTINGS, LV_SYMBOL_SETTINGS },
};
#define NAV_ITEM_COUNT (sizeof(s_items) / sizeof(s_items[0]))

static lv_obj_t *s_bar = NULL;
static lv_obj_t *s_btn[NAV_ITEM_COUNT];

/* ------------------------------------------------------------------ */

static void on_item_clicked(lv_event_t *e)
{
    ui_screens_t target  = (ui_screens_t)(uintptr_t)lv_event_get_user_data(e);
    ui_screens_t current = ui_screens_get_current();

    if (target == current) return;
    if (lv_disp_get_default()->scr_to_load != NULL) return;   /* load in progress */

    lv_scr_load_anim_t anim = (target > current) ? LV_SCR_LOAD_ANIM_MOVE_LEFT
                                                 : LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    ui_screens_navigate_anim(target, anim, UI_SCREENS_ANIM_MS);
}

/* ------------------------------------------------------------------ */

void nav_bar_create(void)
{
    if (s_bar) return;

    s_bar = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_bar, lv_pct(100), NAV_BAR_HEIGHT);
    lv_obj_align(s_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(s_bar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_60, 0);
    lv_obj_set_style_border_width(s_bar, 0, 0);
    lv_obj_set_style_radius(s_bar, 0, 0);
    lv_obj_set_style_pad_all(s_bar, 0, 0);
    lv_obj_set_flex_flow(s_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_bar, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    /* Not clickable: touches between the items fall through to the screen */
    lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < NAV_ITEM_COUNT; i++) {
        lv_obj_t *btn = lv_label_create(s_bar);
        lv_label_set_text_static(btn, s_items[i].symbol);
        lv_obj_set_style_pad_hor(btn, 24, 0);
        lv_obj_set_style_pad_ver(btn, 8, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, on_item_clicked, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)s_items[i].screen);
        s_btn[i] = btn;
    }

    nav_bar_set_active(ui_screens_get_current());
}

void nav_bar_set_active(ui_screens_t screen)
{
    if (!s_bar) return;

    bool visible = false;
    for (size_t i = 0; i < NAV_ITEM_COUNT; i++) {
        bool active = (s_items[i].screen == screen);
        visible |= active;
        lv_color_t c = active ? lv_theme_get_color_primary(s_btn[i])
                              : lv_palette_main(LV_PALETTE_GREY);
        lv_obj_set_style_text_color(s_btn[i], c, 0);
    }

    /* Splash (or unknown screen): no bar */
    if (visible) {
        lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
    }
}
