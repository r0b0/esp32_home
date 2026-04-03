#ifndef APP_H
#define APP_H

#include <Arduino.h>

struct AppStruct {
  lv_obj_t *main_screen;
  lv_obj_t *radio_screen;
  lv_obj_t *radio_status_label;
  lv_obj_t *radio_dropdown;
  unsigned long ticker;
};


char *copyString(String s);

#endif
