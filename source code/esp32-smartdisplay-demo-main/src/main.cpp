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
        8196,                 // Stack size
        NULL,                 // Parameter
        1,                    // Prioriteta
        &displayTaskHandle,   // Task handle
        0                     // Core 0
    );
    delay(100);
    xTaskCreatePinnedToCore(
        Task2,   // Funkcija
        "Data Processing",    // Ime
        8196,                 // Stack size (prilagodi po potrebi)
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
    while (true) {
        updateDisplay();
        delay(5);
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
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
    }
    
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
                const char* razred = obj["razred"].as<const char*>();
                int ura = obj["ura"].as<int>();
                const char* naziv = obj["naziv"].as<const char*>();
                const char* profesor = obj["profesor"].as<const char*>();
            
                Serial.print("Razred: ");
                Serial.print(razred);
                Serial.print(", Ura: ");
                Serial.print(ura);
                Serial.print(", Predmet: ");
                Serial.println(naziv);
            
                setText(ui_Predmet, naziv);
                setText(ui_Profesor, profesor);
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