#ifndef APP_H
#define APP_H

#include <Arduino.h>

struct AppScreen {
  lv_obj_t *screen;
  lv_obj_t *main_flex;
};

struct AppStruct {
  lv_obj_t *main_screen;
  struct AppScreen *radio_screen;
  lv_obj_t *radio_status_label;
  lv_obj_t *radio_dropdown;
  struct AppScreen *weather_screen;
  struct AppScreen *bus_screen;
  unsigned long ticker;
};

char *copy_string(String s);

#endif
