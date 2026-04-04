/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen - Note: this library doesn't require further configuration

#include <WiFi.h>
#include "wifi_creds.h"

// Install the "HttpClient" library by Adrian McEwen

#include "app.h"
#include "radio.h"
#include "hw.h"
#include "gui.h"

// Main app
struct AppStruct app;

static void event_handler_screen_load_btn(lv_event_t * e) {
  lv_screen_load_anim((lv_obj_t *)e->user_data, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 200, 100, false);
}

void create_main_gui(void) {
  // TODO standardize the screens
  // https://medium.com/@akimik/let-item-fill-available-space-in-lvgl-flex-64e0f9e32c9b
  app.main_screen = lv_obj_create(NULL);
  lv_obj_set_flex_flow(app.main_screen, LV_FLEX_FLOW_COLUMN);
  app.radio_screen = gui_make_screen("Radio");
  lv_screen_load(app.main_screen);

  lv_obj_t *wifi_conn_label = lv_label_create(app.main_screen);
  lv_label_set_long_mode(wifi_conn_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(wifi_conn_label, WiFi.status() == WL_CONNECTED ? LV_SYMBOL_WIFI "Wifi Connected" : LV_SYMBOL_CLOSE "Wifi Not Connected");
  lv_obj_set_width(wifi_conn_label, LV_PCT(100));
  lv_obj_set_style_text_align(wifi_conn_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(wifi_conn_label, LV_ALIGN_CENTER, 0, -90);
  
  lv_obj_t *radio_btn = gui_make_btn(app.main_screen, LV_SYMBOL_AUDIO " Radio", LV_PALETTE_BLUE);
  lv_obj_add_event_cb(radio_btn, event_handler_screen_load_btn, LV_EVENT_CLICKED, app.radio_screen->screen);
  
  app.radio_status_label = lv_label_create(app.radio_screen->main_flex);
  lv_label_set_long_mode(app.radio_status_label, LV_LABEL_LONG_MODE_SCROLL);
  lv_label_set_text(app.radio_status_label, "Loading...");
  lv_obj_set_width(wifi_conn_label, LV_PCT(100));

  app.radio_dropdown = lv_dropdown_create(app.radio_screen->main_flex);
  char *r = radios_dropdown_char();
  int sri = selected_radio_index();
  LV_LOG_USER("Radios for dropdown: %s selected: %d", r, sri);
  lv_dropdown_set_options(app.radio_dropdown, r);
  // free(r); // XXX docs say this can be freed but apparently not
  lv_dropdown_set_selected(app.radio_dropdown, sri);
  lv_obj_set_width(app.radio_dropdown, LV_PCT(100));

  lv_obj_t *play_btn = gui_make_btn(app.radio_screen->main_flex, LV_SYMBOL_PLAY " Play", LV_PALETTE_GREEN);
  lv_obj_add_event_cb(play_btn, event_handler_radio_command, LV_EVENT_CLICKED, (void *)"play");

  lv_obj_t *stop_btn = gui_make_btn(app.radio_screen->main_flex, LV_SYMBOL_STOP " Stop", LV_PALETTE_AMBER);
  lv_obj_add_event_cb(stop_btn, event_handler_radio_command, LV_EVENT_CLICKED, (void *)"stop");
}

void connect_to_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() < startAttempt + 10000) {
    delay(500);
  }
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  connect_to_wifi();
  LV_LOG_USER(WiFi.status() == WL_CONNECTED ? "Wifi connected" : "Wifi not connected");

  setup_hw();
  
  fetch_radios();

  create_main_gui();

  app.ticker = millis();
}

void loop_10s() {
  // LV_LOG_USER("tick %d", app.ticker);
  fetch_radio_status();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass

  if(millis() > app.ticker + 10000) {
    app.ticker = millis();
    loop_10s();
  }
}
