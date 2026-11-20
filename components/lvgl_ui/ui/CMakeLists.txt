SET(SOURCES screens/ui_scrSplash.c
    screens/ui_scrHome.c
    screens/ui_scrSettings.c
    screens/ui_scrSystem.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_logotipo_nuevo_png.c)

add_library(ui ${SOURCES})
