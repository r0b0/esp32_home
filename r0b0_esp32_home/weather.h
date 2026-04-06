#ifndef WEATHER_H
#define WEATHER_H

#include <lvgl.h>
#include <ArduinoJson.h>
#include "weather_creds.h"

#define WEATHER_URL "http://api.worldweatheronline.com/premium/v1/weather.ashx?q=Bratislava&format=json&num_of_days=3&extra=localObsTime&mca=no&tp=12&showlocaltime=yes&alerts=yes&key=" WEATHER_KEY

extern struct AppStruct app;

void fetch_weather();
void event_handler_weather_refresh(lv_event_t *e);

#endif