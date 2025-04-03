SET(SOURCES screens/ui_main.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_H1.c
    fonts/ui_font_P1.c
    fonts/ui_font_h2.c
    fonts/ui_font_h3.c
    fonts/ui_font_p.c)

add_library(ui ${SOURCES})
