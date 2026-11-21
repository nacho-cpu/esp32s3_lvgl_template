#include <inttypes.h>

#include "scr_system.h"
#include "screens.h"
#include "ui.h"
#include "esp_system.h"
#include "esp_idf_version.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_private/esp_clk.h"
#include "esp_log.h"

static const char *TAG = "scr_system";

#define SYSTEM_REFRESH_MS 1000

/*
 * Widget names come from the SquareLine export: the info panel is ui_Panel2
 * and the chip label is ui_LblChip_.
 */

static lv_timer_t *s_timer = NULL;

/*
 * Resettable minimum free heap. esp_get_minimum_free_heap_size() is the
 * minimum since boot and cannot be reset, so track our own: sampled every
 * second by a timer that runs even while this screen is hidden, and reset to
 * the current free heap by the physical button.
 */
static uint32_t s_heap_min = UINT32_MAX;

/* ------------------------------------------------------------------ */
/* Labels                                                              */
/* ------------------------------------------------------------------ */

static const char *chip_model_name(esp_chip_model_t model)
{
    switch (model) {
    case CHIP_ESP32:   return "ESP32";
    case CHIP_ESP32S2: return "ESP32-S2";
    case CHIP_ESP32S3: return "ESP32-S3";
    case CHIP_ESP32C3: return "ESP32-C3";
    default:           return "ESP32-?";
    }
}

/** Data that never changes: written once in prepare(). */
static void fill_static_info(void)
{
    lv_label_set_text_fmt(ui_LblIdfVer, "IDF: %s", esp_get_idf_version());

    esp_chip_info_t info;
    esp_chip_info(&info);
    lv_label_set_text_fmt(ui_LblChip_, "Chip: %s rev v%d.%d, %d nucleos",
                          chip_model_name(info.model),
                          info.revision / 100, info.revision % 100,
                          info.cores);

    uint32_t flash_size = 0;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        lv_label_set_text_fmt(ui_LblFlash, "Flash: %" PRIu32 " MB", flash_size / (1024 * 1024));
    } else {
        lv_label_set_text_static(ui_LblFlash, "Flash: --");
    }
}

static void heap_min_sample(void)
{
    uint32_t free_now = esp_get_free_heap_size();
    if (free_now < s_heap_min) {
        s_heap_min = free_now;
    }
}

static void refresh_dynamic(void)
{
    lv_label_set_text_fmt(ui_LblCpuFreq, "CPU: %d MHz", esp_clk_cpu_freq() / 1000000);
    lv_label_set_text_fmt(ui_LblHeapFree, "Heap libre: %" PRIu32 " KB",
                          esp_get_free_heap_size() / 1024);
    lv_label_set_text_fmt(ui_LblHeapMin, "Heap minimo: %" PRIu32 " KB", s_heap_min / 1024);
}

/* ------------------------------------------------------------------ */
/* Timer                                                               */
/* ------------------------------------------------------------------ */

static void system_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    heap_min_sample();          /* always, so the minimum is never missed */
    if (ui_screens_get_current() == SCREEN_SYSTEM) {
        refresh_dynamic();
    }
}

/** Physical button: reset the heap minimum to the current free heap. */
static void on_key(lv_event_t *e)
{
    if (lv_event_get_key(e) != LV_KEY_ENTER) return;
    s_heap_min = esp_get_free_heap_size();
    ESP_LOGI(TAG, "heap minimum reset to %" PRIu32 " bytes", s_heap_min);
    refresh_dynamic();
    ui_screens_flash(ui_LblHeapMin);
}

/* ------------------------------------------------------------------ */
/* Screen controller                                                   */
/* ------------------------------------------------------------------ */

void scr_system_prepare(void)
{
    /* Layout: title on top, info panel below it, bottom 36 px left free for
     * the nav bar. The export has no flex layout on the panel, so set it here. */
    lv_label_set_text_static(ui_LblSystitle, "Sistema");
    lv_obj_align(ui_LblSystitle, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_set_size(ui_Panel2, 300, 150);
    lv_obj_align(ui_Panel2, LV_ALIGN_TOP_MID, 0, 32);
    lv_obj_set_flex_flow(ui_Panel2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Panel2, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_border_width(ui_Panel2, 0, 0);
    lv_obj_set_style_pad_all(ui_Panel2, 8, 0);
    lv_obj_set_style_pad_row(ui_Panel2, 4, 0);
    lv_obj_clear_flag(ui_Panel2, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    /* Same order as the data table in the assignment plan */
    lv_obj_move_to_index(ui_LblCpuFreq,  0);
    lv_obj_move_to_index(ui_LblHeapFree, 1);
    lv_obj_move_to_index(ui_LblHeapMin,  2);
    lv_obj_move_to_index(ui_LblIdfVer,   3);
    lv_obj_move_to_index(ui_LblChip_,    4);
    lv_obj_move_to_index(ui_LblFlash,    5);

    fill_static_info();
    heap_min_sample();
    refresh_dynamic();

    s_timer = lv_timer_create(system_timer_cb, SYSTEM_REFRESH_MS, NULL);

    lv_obj_add_event_cb(ui_scrSystem, on_key, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(ui_scrSystem, ui_screens_gesture_cb, LV_EVENT_GESTURE, NULL);
}

void scr_system_init(void)
{
    ESP_LOGD(TAG, "init");
    heap_min_sample();
    refresh_dynamic();          /* do not wait up to 1 s for fresh values */
}

void scr_system_uninit(void)
{
    ESP_LOGD(TAG, "uninit");
}

void scr_system_step(void)
{
}
