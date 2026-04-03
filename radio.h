#ifndef RADIO_H
#define RADIO_H

#include <lvgl.h>

#include "app.h"

extern struct AppStruct app;
void event_handler_radio_command(lv_event_t *e);
void fetchRadioStatus();

#endif