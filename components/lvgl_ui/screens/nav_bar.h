#ifndef __NAV_BAR_H__
#define __NAV_BAR_H__

#include "screens.h"

/** Height reserved at the bottom of home, system and settings. */
#define NAV_BAR_HEIGHT 36

/**
 * Persistent bottom navigation bar.
 *
 * Created once on lv_layer_top(), so it stays above every screen and is not
 * rebuilt on screen changes. Tapping an item navigates to that screen.
 * LVGL context only.
 */
void nav_bar_create(void);

/** Highlight the item of the active screen; hide the bar on the splash. */
void nav_bar_set_active(ui_screens_t screen);

#endif
