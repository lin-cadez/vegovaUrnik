SET(SOURCES screens/ui_main.c
    screens/ui_startup.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_diapozitiv1_png.c
    images/ui_img_diapozitiv10_png.c
    images/ui_img_diapozitiv11_png.c
    images/ui_img_diapozitiv12_png.c
    images/ui_img_diapozitiv13_png.c
    images/ui_img_diapozitiv14_png.c
    images/ui_img_diapozitiv15_png.c
    images/ui_img_diapozitiv16_png.c
    images/ui_img_diapozitiv17_png.c
    images/ui_img_diapozitiv18_png.c
    images/ui_img_diapozitiv19_png.c
    images/ui_img_diapozitiv2_png.c
    images/ui_img_diapozitiv20_png.c
    images/ui_img_diapozitiv21_png.c
    images/ui_img_diapozitiv3_png.c
    images/ui_img_diapozitiv4_png.c
    images/ui_img_diapozitiv5_png.c
    images/ui_img_diapozitiv6_png.c
    images/ui_img_diapozitiv7_png.c
    images/ui_img_diapozitiv8_png.c
    images/ui_img_diapozitiv9_png.c
    fonts/ui_font_H1.c
    fonts/ui_font_P1.c
    fonts/ui_font_h2.c
    fonts/ui_font_h3.c
    fonts/ui_font_p.c)

add_library(ui ${SOURCES})
