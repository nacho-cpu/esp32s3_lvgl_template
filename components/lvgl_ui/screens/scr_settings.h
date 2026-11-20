#ifndef __SCR_SETTINGS_H__
#define __SCR_SETTINGS_H__

/**
 * Settings screen controller. See scr_home.h for the controller convention.
 * All functions run in LVGL context.
 */
void scr_settings_prepare(void);
void scr_settings_init(void);
void scr_settings_uninit(void);
void scr_settings_step(void);

#endif
