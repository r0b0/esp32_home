#include "mhd.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "socketio.h"

SocketIo socket_io(MHD_URL);

void process_mhd_message(String message) {
  JsonDocument doc;
  DeserializationError json_error = deserializeJson(doc, message);
  if(json_error) {
    Serial.println(json_error.c_str());
    return;
  }
  String event_type = doc[0].as<String>();
  Serial.printf("event type: %s\n", event_type.c_str());
  if(!event_type.equals("tabs")) {
    return;
  }
  JsonArray tabs = doc[1].as<JsonArray>();
  for(JsonObject tab: tabs) {
    int zastavka = tab["zastavka"];
    int nastupiste = tab["nastupiste"];
    Serial.printf(" zastavka %d nastupiste %d\n", zastavka, nastupiste);

    JsonArray tabula = tab["tab"].as<JsonArray>();
    for(JsonObject bus: tabula) {
      const char *linka = bus["linka"];
      const char *ciel = bus["cielStr"];
      const char *odjazd = bus["odjazd"];
      Serial.printf("    %s %s %s\n", linka, ciel, odjazd);
    }
  }
}

int mhd_connect() {
  int err;
  socket_io.set_event_callback(process_mhd_message);
  err = socket_io.connect("");
  if(err) {
    Serial.printf("Socket connect failed: %d\n", err);
    return err;
  }

  err = socket_io.send_event("[\"tabStart\",[355,[864],\"ba\"]]");
  if(err) {
    Serial.printf("Tabs start request failed: %d\n", err);
    return err;
  }

  err = socket_io.receive();
  if(err) {
    Serial.printf("Tabs receive failed: %d\n", err);
    return err;
  }
  return 0;
}

void mhd_loop() {
  socket_io.loop();
}