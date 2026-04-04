#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>
#include <lvgl.h>

#include "app.h"

#define RADIO_URL String("https://radio.lamac.cc/")

struct RadioStruct {
  bool selected;
  char *url;
  char *name;
  struct RadioStruct *next;
};

extern struct AppStruct app;
void event_handler_radio_command(lv_event_t *e);
void fetch_radio_status();
void fetch_radios();
struct RadioStruct *parse_radios(String s);
char *radios_dropdown_char();
int selected_radio_index();
const char *selected_radio();

#endif
