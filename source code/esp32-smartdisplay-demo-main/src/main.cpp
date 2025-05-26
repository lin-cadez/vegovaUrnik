#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "Drop It Like It's HotSpot";
const char* password = "krokituna";
const char* st_ucilnice = "115";

// za osveževanje urnika
unsigned long lastRefresh = 0;
const unsigned long refreshInterval = 2000; 

String serverName = "http://nether.mojvegovc.si:3000";
HTTPClient http;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

char buf[16];
char buf2[16];
char buf3[16];
ulong next_millis;
auto lv_last_tick = millis();

TaskHandle_t displayTaskHandle;
TaskHandle_t task2Handle;

void updateDisplay();
void Task2(void *parameter);
void initDisplay();
void setText(lv_obj_t * obj, const char * text);
void updateDisplayTask(void* parameter);
void refreshUrnik();
lv_obj_t* createLessonBox(const char* subject, const char* teacher, uint8_t lessonNumber, const char* timeRange);
void setup() {
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
    lv_timer_handler();

    //task damo na core 0, za updejtanje displaya
    xTaskCreatePinnedToCore(
        updateDisplayTask,    // Funkcija
        "Display Task",       // Ime
        131072,                 // Stack size
        NULL,                 // Parameter
        1,                    // Prioriteta
        &displayTaskHandle,   // Task handle
        0                     // Core 0
    );
    delay(100);
    xTaskCreatePinnedToCore(
        Task2,   // Funkcija
        "Data Processing",    // Ime
        32768,                 // Stack size (prilagodi po potrebi)
        NULL,                 // Parameter
        1,                    // Prioriteta
        &task2Handle, // Task handle
        1                     // Core 1
    );
    delay(100);
    Serial.println("Data processing task created");
}
//osveževanje zaslona na core 0
void updateDisplayTask(void* parameter) {
    const TickType_t xDelay = pdMS_TO_TICKS(5);
    while (true) {
        uint32_t now = millis();
        uint32_t elapsed = now - lv_last_tick;
        lv_tick_inc(elapsed);
        lv_last_tick = now;

        lv_timer_handler();

        vTaskDelay(xDelay);
    }
}


//preloženo na core 0
void updateDisplay() {
    auto const now = millis();
    if (now < next_millis) {
        return;
    }

    lv_tick_inc(now - lv_last_tick);
    lv_last_tick = now;
    lv_timer_handler();
}

// preloženo na core 1
void Task2(void *parameter) {
    initDisplay();
    Serial.println("Task 2 started");
    while(true){
        loop();
    }
}

bool urnikSetup = false;
bool refreshed = false;
void initDisplay() {
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    while (!urnikSetup) {
        String serverPath = serverName + "/ucilnica/" + st_ucilnice;
        http.begin(serverPath.c_str());
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println("dobil sem");
            urnikSetup = true;
            http.end();
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
    }
    struct tm timeinfo;
    while(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        sprintf(buf2, "%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
        
        setText(ui_datum, buf2);
        setText(ui_TrenutniCas, buf);    
    }
    Serial.println(buf2);
    
    lv_screen_load(ui_main);
    setText(ui_StUcilnice, st_ucilnice);
}

void loop() {
    unsigned long now = millis();
    if (now - lastRefresh >= refreshInterval) {
        lastRefresh = now;
        refreshUrnik();
        refreshed = false;
  }
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
    }
    
    sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    sprintf(buf2, "%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
    
    setText(ui_TrenutniCas, buf);
    setText(ui_datum, buf2);
    
    Serial.println(String(buf));
    delay(500); // Zmanjšamo obremenitev core 1
}

void refreshUrnik(){
    Serial.println("Refreshing urnik");
    while (!refreshed) {
        String serverPath = serverName + "/ucilnica/" + st_ucilnice;
        http.begin(serverPath.c_str());
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            http.end();
            
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.print("Deserialization failed: ");
                Serial.println(error.f_str());
                return;
            }
            refreshed = true;
            JsonArray data = doc.as<JsonArray>();
            for (JsonObject obj : data) {
                createLessonBox(
                    obj["predmet"],
                    obj["ucitelj"],
                    obj["st_ure"],
                    obj["cas"]
                );
            }

            urnikSetup = true;
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        delay(100);
    }
}

void setText(lv_obj_t * obj, const char * text) {
    lv_label_set_text(obj, text);
    lv_obj_set_style_text_font(obj, &ui_font_H1, LV_PART_MAIN | LV_STATE_DEFAULT);
}
lv_obj_t* createLessonBox(const char* subject, const char* teacher, uint8_t lessonNumber, const char* timeRange) {
    lv_obj_t* box = lv_obj_create(ui_timetableContainer);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, lv_pct(100), 100);
    lv_obj_set_pos(box, -186, -325);
    lv_obj_set_flex_flow(box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(box, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(box, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(box, 25, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* infoContainer = lv_obj_create(box);
    lv_obj_remove_style_all(infoContainer);
    lv_obj_set_size(infoContainer, 200, 70);
    lv_obj_set_align(infoContainer, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(infoContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(infoContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(infoContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(infoContainer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* circle = lv_obj_create(infoContainer);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, 40, 40);
    lv_obj_set_align(circle, LV_ALIGN_CENTER);
    lv_obj_add_state(circle, LV_STATE_CHECKED);
    lv_obj_set_style_radius(circle, 1000, LV_PART_MAIN);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xF00), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(circle, 255, LV_PART_MAIN);

    lv_obj_t* labelNum = lv_label_create(circle);
    lv_label_set_text_fmt(labelNum, "%d", lessonNumber);
    lv_obj_center(labelNum);
    lv_obj_set_style_text_color(labelNum, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(labelNum, &ui_font_h3, LV_PART_MAIN);

    lv_obj_t* moreInfo = lv_obj_create(infoContainer);
    lv_obj_remove_style_all(moreInfo);
    lv_obj_set_size(moreInfo, 127, 65);
    lv_obj_set_flex_flow(moreInfo, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(moreInfo, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(moreInfo, 8, LV_PART_MAIN);

    lv_obj_t* topInfo = lv_obj_create(moreInfo);
    lv_obj_remove_style_all(topInfo);
    lv_obj_set_size(topInfo, lv_pct(123), 34);
    lv_obj_set_flex_flow(topInfo, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topInfo, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(topInfo, 10, LV_PART_MAIN);

    lv_obj_t* subjectLabel = lv_label_create(topInfo);
    lv_label_set_text(subjectLabel, subject);
    lv_obj_set_style_text_color(subjectLabel, lv_color_hex(0xF00), LV_PART_MAIN);
    lv_obj_set_style_text_font(subjectLabel, &ui_font_h3, LV_PART_MAIN);

    lv_obj_t* gradeContainer = lv_obj_create(topInfo);
    lv_obj_remove_style_all(gradeContainer);
    lv_obj_set_style_radius(gradeContainer, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(gradeContainer, lv_color_hex(0xF00), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gradeContainer, 255, LV_PART_MAIN);
    lv_obj_set_style_pad_all(gradeContainer, 5, LV_PART_MAIN);

    lv_obj_t* gradeLabel = lv_label_create(gradeContainer);
    lv_label_set_text(gradeLabel, "G3A");
    lv_obj_set_style_text_color(gradeLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(gradeLabel, &ui_font_P1, LV_PART_MAIN);

    lv_obj_t* teacherLabel = lv_label_create(moreInfo);
    lv_label_set_text(teacherLabel, teacher);
    lv_obj_set_style_text_color(teacherLabel, lv_color_hex(0x777777), LV_PART_MAIN);
    lv_obj_set_style_text_font(teacherLabel, &ui_font_p, LV_PART_MAIN);

    lv_obj_t* timeContainer = lv_obj_create(box);
    lv_obj_remove_style_all(timeContainer);

    lv_obj_t* timeLabel = lv_label_create(timeContainer);
    lv_label_set_text(timeLabel, timeRange);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_14, LV_PART_MAIN);

    return box;
}