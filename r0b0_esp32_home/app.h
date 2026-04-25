#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <lvgl.h>

struct AppScreen {
  lv_obj_t *screen;
  lv_obj_t *top_flex;
  lv_obj_t *main_flex;
};

struct AppStruct {
  lv_obj_t *main_screen;
  lv_obj_t *main_screen_last_btn;
  struct AppScreen *radio_screen;
  lv_obj_t *radio_status_label;
  lv_obj_t *radio_dropdown;
  struct AppScreen *weather_screen;
  lv_obj_t *weather_label;
  struct AppScreen *bus_screen;
  lv_obj_t *bus_label;
  unsigned long ticker;
  bool turned_on;
  unsigned long last_touch;
};

extern struct AppStruct app;

char *copy_string(String s);

#endif
