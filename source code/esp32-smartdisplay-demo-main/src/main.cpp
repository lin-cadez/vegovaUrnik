#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid            = "Drop It Like It's HotSpot";
const char* password        = "krokituna";
const char* st_ucilnice     = "115";
String      serverName      = "http://nether.mojvegovc.si:3000";
HTTPClient  http;

const char* ntpServer         = "pool.ntp.org";
const long  gmtOffset_sec     = 3600;
const int   daylightOffset_sec = 3600;

// arrays of pointers to the 9 lesson‐slot widgets
lv_obj_t* containers[9];
lv_obj_t* profesor[9];
lv_obj_t* ura[9];
lv_obj_t* razred[9];
lv_obj_t* predmet[9];

auto lv_last_tick = millis();

void initDisplay();
void refreshUrnik();
void setText(lv_obj_t* obj, const char* txt);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Basic board info
  log_i("Board: %s",       BOARD_NAME);
  log_i("CPU: %s rev%d",    ESP.getChipModel(), ESP.getChipRevision());
  log_i("Freq: %d MHz",     getCpuFrequencyMhz());
  log_i("Cores: %d",        ESP.getChipCores());
  log_i("Heap: %d bytes",   ESP.getFreeHeap());
  log_i("PSRAM: %d bytes",  ESP.getPsramSize());
  log_i("SDK ver: %s",      ESP.getSdkVersion());

  // init LVGL display & UI
  smartdisplay_init();
  ui_init();
  lv_disp_set_rotation(lv_disp_get_default(), LV_DISPLAY_ROTATION_90);
  lv_screen_load(ui_startup);

  // now ui_* pointers are valid – populate our arrays
  // topInfo containers:
  containers[0] = ui_topInfo1;  containers[1] = ui_topInfo2;  containers[2] = ui_topInfo3;
  containers[3] = ui_topInfo4;  containers[4] = ui_topInfo5;  containers[5] = ui_topInfo6;
  containers[6] = ui_topInfo7;  containers[7] = ui_topInfo8;  containers[8] = ui_topInfo9;
  // labels for each slot:
  profesor[0] = ui_Profesor;    profesor[1] = ui_Profesor1;   profesor[2] = ui_Profesor2;
  profesor[3] = ui_Profesor3;   profesor[4] = ui_Profesor4;   profesor[5] = ui_Profesor5;
  profesor[6] = ui_Profesor6;   profesor[7] = ui_Profesor7;   profesor[8] = ui_Profesor8;

  ura[0]      = ui_StUre;       ura[1]      = ui_StUre1;      ura[2]      = ui_StUre2;
  ura[3]      = ui_StUre3;      ura[4]      = ui_StUre4;      ura[5]      = ui_StUre5;
  ura[6]      = ui_StUre6;      ura[7]      = ui_StUre7;      ura[8]      = ui_StUre8;

  razred[0]   = ui_Razred;      razred[1]   = ui_Razred1;     razred[2]   = ui_Razred2;
  razred[3]   = ui_Razred3;     razred[4]   = ui_Razred4;     razred[5]   = ui_Razred5;
  razred[6]   = ui_Razred6;     razred[7]   = ui_Razred7;     razred[8]   = ui_Razred8;

  predmet[0]  = ui_Predmet;     predmet[1]  = ui_Predmet1;    predmet[2]  = ui_Predmet2;
  predmet[3]  = ui_Predmet3;    predmet[4]  = ui_Predmet4;    predmet[5]  = ui_Predmet5;
  predmet[6]  = ui_Predmet6;    predmet[7]  = ui_Predmet7;    predmet[8]  = ui_Predmet8;

  // Debug: confirm pointers
  for (int i = 0; i < 9; i++) {
    Serial.printf("slot %d container=%p prop=%p ura=%p raz=%p pred=%p\n",
                  i+1,
                  (void*)containers[i],
                  (void*)profesor[i],
                  (void*)ura[i],
                  (void*)razred[i],
                  (void*)predmet[i]
    );
  }

  // show classroom number
  setText(ui_StUcilnice, st_ucilnice);

  // sync time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  lv_timer_handler();

  // bring up Wi-Fi and display
  initDisplay();
}

void loop() {
  uint32_t now = millis();
  lv_tick_inc(now - lv_last_tick);
  lv_last_tick = now;
  lv_timer_handler();
}

void initDisplay() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" WiFi connected!");
  refreshUrnik();
  lv_screen_load(ui_main);
  setText(ui_StUcilnice, st_ucilnice);
}

void refreshUrnik() {
  Serial.println("Refreshing urnik…");

  String url = serverName + "/ucilnica/" + st_ucilnice;
  http.begin(url.c_str());
  int code = http.GET();
  if (code <= 0) {
    Serial.printf("HTTP error %d\n", code);
    http.end();
    return;
  }

  Serial.printf("HTTP response: %d\n", code);
  String payload = http.getString();
  Serial.println("Payload: " + payload);
  http.end();

  DynamicJsonDocument doc(4096);
  auto err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("JSON err: ");
    Serial.println(err.f_str());
    return;
  }
  JsonArray data = doc.as<JsonArray>();
  Serial.printf("Items: %u\n", data.size());

  // figure out today: Mon=0 … Sun=6
  time_t t = time(nullptr);
  struct tm tmbuf;
  localtime_r(&t, &tmbuf);
  int today = tmbuf.tm_wday;    // Sun=0 … Sat=6
  if (today == 0) today = 7;    // Sunday→7
  today -= 1;                   // shift→Mon=0…

  Serial.printf("Today index: %d\n", today);

  // clear/hide every slot first
  for (int i = 0; i < 9; i++) {
    // hide container
    lv_obj_add_flag(containers[i], LV_OBJ_FLAG_HIDDEN);
    // set professor label to "Prazno"
    lv_label_set_text(profesor[i], "Prazno");
    // clear others
    lv_label_set_text(razred[i], "");
    lv_label_set_text(predmet[i], "");
  }

  // place each lesson into its period-slot (hour→idx = hour-1)
  for (JsonObject obj : data) {
    int dan  = obj["dan"].as<int>();
    int uraH = obj["ura"].as<int>();
    if (dan != today) continue;
    if (uraH < 1 || uraH > 9) continue;

    int idx = uraH - 1;
    // unhide
    lv_obj_clear_flag(containers[idx], LV_OBJ_FLAG_HIDDEN);
    // populate
    lv_label_set_text(ura[idx], String(uraH).c_str());
    lv_label_set_text(razred[idx], obj["razred"] | "");
    lv_label_set_text(predmet[idx], obj["naziv"]   | "");
    lv_label_set_text(profesor[idx], obj["profesor"]| "");

    Serial.printf("Slot %d ← %s %s (%s)\n",
      uraH,
      (obj["razred"]   | "?"),
      (obj["naziv"]    | "?"),
      (obj["profesor"] | "?")
    );
  }
}

void setText(lv_obj_t* obj, const char* txt) {
  if (!obj) return;
  lv_label_set_text(obj, txt);
  lv_obj_set_style_text_font(obj, &ui_font_H1, LV_PART_MAIN | LV_STATE_DEFAULT);
}
