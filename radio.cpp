#include <Arduino.h>
#include <lvgl.h>
#include <HTTPClient.h>

#include "app.h"
#include "radio.h"

struct RadioStruct *knownRadios=NULL;

void event_handler_radio_command(lv_event_t *e) {
  String command = String((char *)e->user_data);
  String radio = selectedRadio();
  String url = RADIO_URL + "command";
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Accept", "text/plain");
  String data = "radio=" + radio + "&" + command + "=";
  LV_LOG_USER("Posting %s to %s", data.c_str(), url.c_str());
  int httpCode = http.POST(data);
  LV_LOG_USER("Play post return code %d", httpCode);
  http.end();
}

void fetchRadios() {
  String url = RADIO_URL + "radio";
  HTTPClient http;
  http.begin(url);
  http.addHeader("Accept", "text/plain");
  int httpCode = http.GET();
  if (httpCode == 200) {
    String radios = http.getString();
    LV_LOG_USER("Fetched radios %s", radios.c_str());
    knownRadios = parseRadios(radios);  // TODO: free previous values
  } else {
    LV_LOG_USER("http call failed %d", httpCode);
  }
  http.end();
}

struct RadioStruct *parseRadios(String s) {
  s.trim();
  // LV_LOG_USER("Parsing radios from:");
  // LV_LOG_USER(s.c_str());
  if(s.length() == 0) {
    return NULL;
  }
  struct RadioStruct *r = (struct RadioStruct *)malloc(sizeof(struct RadioStruct));
  r->selected = false;
  int n = s.indexOf("\n");
  String current;
  if(n > 0) {
    // LV_LOG_USER("Newline found at %d", n);
    current = s.substring(0, n);
  } else {
    // LV_LOG_USER("Newline not found");
    current = s;
  }
  // LV_LOG_USER("Current line to process:");
  // LV_LOG_USER(current.c_str());
  int t = current.indexOf("\t");
  if(t < 0) {
    // LV_LOG_USER("Tab not found");
    return NULL;
  }
  // LV_LOG_USER("Tab found at %d", t);
  String url = current.substring(0, t);
  url.trim();
  // TODO: check for one more tab and word "selected"
  String name = current.substring(t);
  name.trim();
  int ts = name.indexOf("\t");
  if(ts > 0) {
    String selectedStr = name.substring(ts);
    selectedStr.trim();
    if(selectedStr.startsWith("selected")) {
      r->selected = true;
    }
    name = name.substring(0, ts);
    name.trim();
  }
  r->url = copyString(url);
  r->name = copyString(name);
  LV_LOG_USER("Parsed url '%s' name '%s' selected %d",
    r->url, r->name, r->selected);

  if(n > 0) {
    r->next = parseRadios(s.substring(n));
  } else {
    r->next = NULL;
  }
  return r;
}

String radiosDropdown(struct RadioStruct *r) {
  // LV_LOG_USER("checking radios dropdown");
  if(r->next == NULL) {
    return String(r->name);
  }
  return String(r->name) + String("\n") + radiosDropdown(r->next);
}

char *radiosDropdownChar() {
  // LV_LOG_USER("Checking radios dropdown as char*");
  return copyString(radiosDropdown(knownRadios));
}

const char *selectedRadio() {
  int i = lv_dropdown_get_selected(app.radio_dropdown);
  LV_LOG_USER("Radio dropdown selected index: %d", i);
  if(i<0) {
    return NULL;
  }
  struct RadioStruct *s = knownRadios;
  for(int j=0; j<i; j++) {
    s = s->next;
    if(s == NULL) {
      return NULL;
    }
  }
  LV_LOG_USER("Found radio %s", s->name);
  return s->url;
}

int selectedRadioIndex() {
  struct RadioStruct *s = knownRadios;
  for(int j=0;; j++) {
    if(s->selected)
      return j;
    s = s->next;
    if(s == NULL) {
      return 0;
    }
  }
}

void fetchRadioStatus() {
  if(lv_screen_active() != app.radio_screen) {
    // LV_LOG_USER("Not fetching radio status, not on the radio page");
    return;
  }

  String url = RADIO_URL + "status";
  HTTPClient http;
  http.begin(url);
  http.addHeader("Accept", "text/plain");
  int httpCode = http.GET();
  if (httpCode == 200) {
    String status = http.getString();
    lv_label_set_text(app.radio_status_label, status.c_str());
  } else {
    LV_LOG_USER("http call failed %d", httpCode);
  }
  http.end();
}
