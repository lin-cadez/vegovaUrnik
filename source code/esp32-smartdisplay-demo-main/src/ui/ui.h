// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"
#include "ui_theme_manager.h"
#include "ui_themes.h"

void startupAnimation_Animation( lv_obj_t *TargetObject, int delay);
void fadeup_Animation( lv_obj_t *TargetObject, int delay);

// SCREEN: ui_main
void ui_main_screen_init(void);
extern lv_obj_t *ui_main;
extern lv_obj_t *ui_NavBar;
extern lv_obj_t *ui_clockContainer1;
extern lv_obj_t *ui_TrenutniCas;
extern lv_obj_t *ui_StUcilnice;
extern lv_obj_t *ui_clockContainer2;
extern lv_obj_t *ui_datum;
extern lv_obj_t *ui_timetableContainer;
extern lv_obj_t *ui_lessonBox;
extern lv_obj_t *ui_infoContainer2;
extern lv_obj_t *ui_circle2;
extern lv_obj_t *ui_StUre;
extern lv_obj_t *ui_moreInfoContainer2;
extern lv_obj_t *ui_topInfo1;
extern lv_obj_t *ui_Predmet;
extern lv_obj_t *ui_Container1;
extern lv_obj_t *ui_Razred;
extern lv_obj_t *ui_Profesor;
extern lv_obj_t *ui_Container3;
extern lv_obj_t *ui_UraPredmeta;
extern lv_obj_t *ui_lessonBoxEmpty;
extern lv_obj_t *ui_infoContainer1;
extern lv_obj_t *ui_circle1;
extern lv_obj_t *ui_StUre1;
extern lv_obj_t *ui_moreInfoContainer1;
extern lv_obj_t *ui_Predmet1;
extern lv_obj_t *ui_Container2;
extern lv_obj_t *ui_UraPredmeta1;
// CUSTOM VARIABLES

// SCREEN: ui_startup
void ui_startup_screen_init(void);
extern lv_obj_t *ui_startup;
extern lv_obj_t *ui_Label19;
extern lv_obj_t *ui_imageContainer;
extern lv_obj_t *ui_Spinner1;
// CUSTOM VARIABLES

// EVENTS
extern lv_obj_t *ui____initial_actions0;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE( ui_img_diapozitiv1_png);   // assets/Diapozitiv1.png
LV_IMG_DECLARE( ui_img_diapozitiv10_png);   // assets/Diapozitiv10.png
LV_IMG_DECLARE( ui_img_diapozitiv11_png);   // assets/Diapozitiv11.png
LV_IMG_DECLARE( ui_img_diapozitiv12_png);   // assets/Diapozitiv12.png
LV_IMG_DECLARE( ui_img_diapozitiv13_png);   // assets/Diapozitiv13.png
LV_IMG_DECLARE( ui_img_diapozitiv14_png);   // assets/Diapozitiv14.png
LV_IMG_DECLARE( ui_img_diapozitiv15_png);   // assets/Diapozitiv15.png
LV_IMG_DECLARE( ui_img_diapozitiv16_png);   // assets/Diapozitiv16.png
LV_IMG_DECLARE( ui_img_diapozitiv17_png);   // assets/Diapozitiv17.png
LV_IMG_DECLARE( ui_img_diapozitiv18_png);   // assets/Diapozitiv18.png
LV_IMG_DECLARE( ui_img_diapozitiv19_png);   // assets/Diapozitiv19.png
LV_IMG_DECLARE( ui_img_diapozitiv2_png);   // assets/Diapozitiv2.png
LV_IMG_DECLARE( ui_img_diapozitiv20_png);   // assets/Diapozitiv20.png
LV_IMG_DECLARE( ui_img_diapozitiv21_png);   // assets/Diapozitiv21.png
LV_IMG_DECLARE( ui_img_diapozitiv3_png);   // assets/Diapozitiv3.png
LV_IMG_DECLARE( ui_img_diapozitiv4_png);   // assets/Diapozitiv4.png
LV_IMG_DECLARE( ui_img_diapozitiv5_png);   // assets/Diapozitiv5.png
LV_IMG_DECLARE( ui_img_diapozitiv6_png);   // assets/Diapozitiv6.png
LV_IMG_DECLARE( ui_img_diapozitiv7_png);   // assets/Diapozitiv7.png
LV_IMG_DECLARE( ui_img_diapozitiv8_png);   // assets/Diapozitiv8.png
LV_IMG_DECLARE( ui_img_diapozitiv9_png);   // assets/Diapozitiv9.png

// FONTS
LV_FONT_DECLARE( ui_font_H1);
LV_FONT_DECLARE( ui_font_P1);
LV_FONT_DECLARE( ui_font_h2);
LV_FONT_DECLARE( ui_font_h3);
LV_FONT_DECLARE( ui_font_p);

// UI INIT
void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
