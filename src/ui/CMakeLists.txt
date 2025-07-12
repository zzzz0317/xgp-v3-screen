SET(SOURCES screens/ui_Boot.c
    screens/ui_Splash.c
    screens/ui_SystemInfo.c
    screens/ui_SystemStatus.c
    screens/ui_NetworkInfo.c
    screens/ui_ModemInfo.c
    screens/ui_ModemSignal.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_581822748.c
    fonts/ui_font_MiSans20.c
    fonts/ui_font_MiSans24.c
    fonts/ui_font_MiSans16.c)

add_library(ui ${SOURCES})
