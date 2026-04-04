/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen - Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>

#include <WiFi.h>
#include "wifi_creds.h"

// Install the "HttpClient" library by Adrian McEwen

#include "app.h"
#include "radio.h"

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Main app
struct AppStruct app;

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    /* Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();*/
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void event_handler_screen_load_btn(lv_event_t * e) {
  lv_screen_load_anim((lv_obj_t *)e->user_data, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 200, 100, false);
}

static void event_handler_back_btn(lv_event_t *e) {
  lv_screen_load_anim(app.main_screen, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 200, 100, false);
}

void lv_create_main_gui(void) {
  // TODO standardize the screens
  // https://medium.com/@akimik/let-item-fill-available-space-in-lvgl-flex-64e0f9e32c9b
  app.main_screen = lv_obj_create(NULL);
  lv_obj_set_flex_flow(app.main_screen, LV_FLEX_FLOW_COLUMN);
  app.radio_screen = make_screen("Radio");
  lv_screen_load(app.main_screen);

  lv_obj_t *wifi_conn_label = lv_label_create(app.main_screen);
  lv_label_set_long_mode(wifi_conn_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(wifi_conn_label, WiFi.status() == WL_CONNECTED ? LV_SYMBOL_WIFI "Wifi Connected" : LV_SYMBOL_CLOSE "Wifi Not Connected");
  lv_obj_set_width(wifi_conn_label, LV_PCT(100));
  lv_obj_set_style_text_align(wifi_conn_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(wifi_conn_label, LV_ALIGN_CENTER, 0, -90);
  
  lv_obj_t *radio_btn = make_btn(app.main_screen, LV_SYMBOL_AUDIO " Radio", LV_PALETTE_BLUE);
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

  lv_obj_t *play_btn = make_btn(app.radio_screen->main_flex, LV_SYMBOL_PLAY " Play", LV_PALETTE_GREEN);
  lv_obj_add_event_cb(play_btn, event_handler_radio_command, LV_EVENT_CLICKED, (void *)"play");

  lv_obj_t *stop_btn = make_btn(app.radio_screen->main_flex, LV_SYMBOL_STOP " Stop", LV_PALETTE_AMBER);
  lv_obj_add_event_cb(stop_btn, event_handler_radio_command, LV_EVENT_CLICKED, (void *)"stop");

}

lv_obj_t *make_btn(lv_obj_t *parent, const char *text, lv_palette_t color) {
  lv_obj_t *b = lv_button_create(parent);
  // lv_obj_set_width(b, LV_PCT(100));
  lv_obj_set_style_bg_color(b, lv_palette_main(color), 0);
  lv_obj_remove_flag(b, LV_OBJ_FLAG_PRESS_LOCK);
  lv_obj_t *label = lv_label_create(b);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  return b;
}

struct AppScreen *make_screen(const char *name) {
  struct AppScreen *screen = (struct AppScreen *)malloc(sizeof(struct AppScreen));

  screen->screen = lv_obj_create(NULL);
  lv_obj_set_layout(screen->screen, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(screen->screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(screen->screen, LV_PCT(100), LV_PCT(100));

  lv_obj_t *top = lv_obj_create(screen->screen);
  lv_obj_set_layout(top, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);

  lv_obj_t *back_btn = make_btn(top, LV_SYMBOL_LEFT " Back", LV_PALETTE_TEAL);
  lv_obj_add_event_cb(back_btn, event_handler_back_btn, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(top);
  lv_label_set_text(label, name);
  lv_obj_set_flex_grow(label, 1);

  screen->main_flex = lv_obj_create(screen->screen);
  lv_obj_set_layout(screen->main_flex, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(screen->main_flex, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_width(screen->main_flex, LV_PCT(100));
  lv_obj_set_flex_grow(screen->main_flex, 1);

  return screen;
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
  Serial.println(WiFi.status() == WL_CONNECTED ? "Wifi connected" : "Wifi not connected");
  
  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);
  
  fetch_radios();

  // Function to draw the GUI (text, buttons and sliders)
  lv_create_main_gui();
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
