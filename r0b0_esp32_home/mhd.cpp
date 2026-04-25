#include "mhd.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "socketio.h"

SocketIo socket_io(MHD_URL);
handle_bus_f mhd_callback;

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
    int i = 0;
    for(JsonObject bus: tabula) {
      String linka = bus["linka"];
      String ciel = bus["cielStr"];
      String odjazd = bus["odjazd"];
      Serial.printf("    %s %s %s\n", linka.c_str(), ciel.c_str(), odjazd.c_str());
      mhd_callback(linka, ciel, odjazd, i == 0, i == tabula.size()-1);
      i++;
    }
  }
}

int mhd_connect(int stop, String platform, handle_bus_f callback) {
  mhd_callback = callback;
  int err;
  socket_io.set_event_callback(process_mhd_message);
  err = socket_io.connect("");
  if(err) {
    Serial.printf("Socket connect failed: %d\n", err);
    return err;
  }
  char tabStart[256];
  sprintf(tabStart, "[\"tabStart\",[%d,[%s],\"ba\"]]", stop, platform);
  err = socket_io.send_event(String(tabStart));
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