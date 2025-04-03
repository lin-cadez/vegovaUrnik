#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>

void OnRotateClicked(lv_event_t *e)
{
    auto disp = lv_disp_get_default();
    auto rotation = (lv_display_rotation_t)((lv_disp_get_rotation(disp) + 1) % (LV_DISPLAY_ROTATION_270 + 1));
    lv_display_set_rotation(disp, rotation);

}

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




    

    
}

ulong next_millis;
auto lv_last_tick = millis();

void loop()
{   


    lv_label_set_text(ui_Label4,"dfsdf");
    lv_obj_set_style_bg_color(ui_circle1, lv_color_hex(0xFFFFFF00), LV_PART_MAIN | LV_STATE_DEFAULT);


    //NIKOLI NE ZBRIŠI!!!!!!
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