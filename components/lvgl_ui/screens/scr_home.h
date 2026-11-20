#ifndef __SCR_HOME_H__
#define __SCR_HOME_H__

/**
 * Home screen controller.
 *
 * Convention for all screen controllers:
 *   prepare()  — called once at startup; register persistent event callbacks
 *                on widgets that should survive screen transitions.
 *   init()     — called each time this screen becomes active; restore state,
 *                start animations, refresh dynamic labels.
 *   uninit()   — called when navigating away; stop timers, free temporaries.
 *   step()     — called every ~20 ms while active; poll sensors, drive
 *                incremental animations, handle encoder input.
 *
 * All four run inside the LVGL timer context (see lvgl_ui.c), so they may
 * call lv_* functions directly.
 */
void scr_home_prepare(void);
void scr_home_init(void);
void scr_home_uninit(void);
void scr_home_step(void);

#endif
