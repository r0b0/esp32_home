#include <Arduino.h>
#include <lvgl.h>
#include <HTTPClient.h>

#include "app.h"
#include "radio.h"


void event_handler_radio_command(lv_event_t *e) {
  String command = String((char *)e->user_data);
  String player = "pi.lamac.cc%3A6600"; // TODO
  String radio = "http%3A%2F%2Fstream.radioparadise.com%2Faac-320"; // TODO
  HTTPClient http;
  http.begin("https://radio.lamac.cc/command");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String data = "player="+ player + "&radio=" + radio + "&" + command + "=";
  LV_LOG_USER("Posting %s to MPD", data);
  int httpCode = http.POST(data);
  LV_LOG_USER("Play post return code %d", httpCode);
}

void fetchRadioStatus() {
  if(lv_screen_active() != app.radio_screen) {
    LV_LOG_USER("Not fetching radio status, not on the radio page");
    return;
  }

  HTTPClient http;
  http.begin("https://radio.lamac.cc/status?player=pi.lamac.cc:6600");
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    payload.trim();
    if(payload.startsWith("<span")) {
      payload = payload.substring(32);
      payload.trim();
    }
    String status = payload.substring(0, payload.indexOf("<"));
    LV_LOG_USER("http call success %s", status.c_str());
    if(status.length()>50) {
      status = status.substring(0, 50);
    }
    lv_label_set_text(app.radio_status_label, status.c_str());
  } else {
    LV_LOG_USER("http call failed %d", httpCode);
  }
  http.end();
}