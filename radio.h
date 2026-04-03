#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>
#include <lvgl.h>

#include "app.h"

#define RADIO_URL String("https://radio.lamac.cc/")

struct RadioStruct {
  char *url;
  char *name;
  struct RadioStruct *next;
};

extern struct AppStruct app;
void event_handler_radio_command(lv_event_t *e);
void fetchRadioStatus();
void fetchRadios();
struct RadioStruct *parseRadios(String s);
const char *radiosDropdownChar();
const char *selectedRadio();

#endif