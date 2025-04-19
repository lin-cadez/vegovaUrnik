#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
#include <WiFi.h>
#include "time.h"


const char* ssid     = "Drop It Like It's HotSpot";
const char* password = "krokituna";
const char* st_ucilnice = "115";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;  // adjust based on your timezone
const int   daylightOffset_sec = 3600; // if you have daylight savings


void updateDisplay();
void initDisplay();
void setText(lv_obj_t * obj, const char * text);


void OnRotateClicked(lv_event_t *e)
{
    auto disp = lv_disp_get_default();
    auto rotation = (lv_display_rotation_t)((lv_disp_get_rotation(disp) + 1) % (LV_DISPLAY_ROTATION_270 + 1));
    lv_display_set_rotation(disp, rotation);

}

char buf[10]; //ura
char buf2[10]; //datum
ulong next_millis;
auto lv_last_tick = millis();

void setup()
{
#ifdef ARDUINO_USB_CDC_ON_BOOT
    delay(5000);
#endif
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_i("Board: %s", BOARD_NAME);
    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    smartdisplay_init();

    __attribute__((unused)) auto disp = lv_disp_get_default();
    ui_init();
    lv_disp_set_rotation(disp, LV_DISPLAY_ROTATION_90);
    lv_screen_load(ui_startup);
    lv_label_set_text(ui_StUcilnice, st_ucilnice);

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.print("Connecting to WiFi");
    lv_timer_handler();
    initDisplay();
    //core 1
}

boolean urnikSetup = true;
void initDisplay(){
    WiFi.begin(ssid, password);
    while((WiFi.status() != WL_CONNECTED) && (urnikSetup)) {
        updateDisplay();
    }
    lv_screen_load(ui_main);
    setText(ui_StUcilnice, st_ucilnice);

    struct tm timeinfo;

    sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    String ura = String(buf);
    sprintf(buf2, "%d.%02d", timeinfo.tm_mon, timeinfo.tm_mday);
    String datum = String(buf2);

    setText(ui_datum, datum.c_str());
    setText(ui_TrenutniCas, ura.c_str());
    
}
void setText(lv_obj_t * obj, const char * text){
    lv_label_set_text(obj, text);
    lv_obj_set_style_text_font(obj, &ui_font_H1, LV_PART_MAIN | LV_STATE_DEFAULT);
}


void updateDisplay(){
    auto const now = millis();
    if (now < next_millis) {
        delay(1);
        return;
    }
    
    // Update the ticker
    lv_tick_inc(now - lv_last_tick);
    lv_last_tick = now;
    // Update the UI
    lv_timer_handler();
}

void loop()
{      
    //core 1
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
    sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    String ura = String(buf);
    sprintf(buf2, "%d.%02d", timeinfo.tm_mon, timeinfo.tm_mday);
    String datum = String(buf2);
    //lv_label_set_text(ui_Label4,"dfsdf");
    //lv_obj_set_style_bg_color(ui_circle1, lv_color_hex(0xFFFFFF00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_TrenutniCas, ura.c_str());
    lv_obj_set_style_text_font(ui_TrenutniCas, &ui_font_H1, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    

    //NIKOLI NE ZBRIŠI!!!!!!
    updateDisplay();
}