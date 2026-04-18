#include "mhd.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

const char *mhd_socketio_sid;

int mhd_connect() {
  HTTPClient http;
  // http.setReuse(false);
  String url = String(MHD_URL) + String("?EIO=4&transport=polling");
  Serial.printf("Connecting to %s\n", url.c_str());
  http.begin(url);
  Serial.printf("getting %s\n", url.c_str());
  int ret = http.GET();
  String received_data = http.getString();
  Serial.printf("connect data: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -1;
  }
  if(!received_data.startsWith("0{")) {
    return -2;
  }
  JsonDocument doc;
  DeserializationError json_error = deserializeJson(doc, received_data.substring(1));
  if(json_error) {
    Serial.println(json_error.c_str());
    return -3;
  }
  mhd_socketio_sid = doc["sid"];
  // http.end();
  // delay(10000);

  url = url + String("&sid=") + String(mhd_socketio_sid);
  http.begin(url);
  Serial.printf("posting 40 to %s\n", url.c_str());
  ret = http.POST(String("40"));
  received_data = http.getString();
  Serial.printf("40 response: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -4;
  }
  if(!received_data.startsWith("ok")) {
    return -5;
  }
  // http.end();
  // delay(10000);
  
  Serial.printf("getting %s\n", url.c_str());
  http.begin(url);
  ret = http.GET();
  received_data = http.getString();
  Serial.printf("get response: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -11;
  }
  // http.end();
  // delay(10000);

  http.begin(url);
  String tabStartData = String("420[\"tabStart\",[355,[864],\"ba\"]]\n");
  Serial.printf("posting %s to %s\n", tabStartData.c_str(), url.c_str());
  ret = http.POST(tabStartData);
  received_data = http.getString();
  Serial.printf("tabstart response: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -6;
  }
  if(!received_data.startsWith("ok")) {
    return -7;
  }
  // http.end();
  // delay(10000);
  
  return mhd_get();
  return 0;
}

int mhd_get() {
  HTTPClient http;
  String url = String(MHD_URL) + String("?EIO=4&transport=polling&sid=" + String(mhd_socketio_sid));
  Serial.printf("getting %s\n", url.c_str());
  http.begin(url);
  int ret = http.GET();
  String received_data = http.getString();
  Serial.printf("get response: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -8;
  }

  if(received_data.indexOf('42["tabs') < 0) {
    return -9;
  }

  String jsonString = received_data.substring(
      received_data.indexOf('42["tabs')+3);
  Serial.println("Deserializing:");
  Serial.println(jsonString.c_str());

  JsonDocument doc;
  DeserializationError json_error = deserializeJson(doc, jsonString);
  if(json_error) {
    Serial.println(json_error.c_str());
    return -10;
  }

  JsonArray tab = doc[0]["tab"].as<JsonArray>();
  for(JsonObject bus: tab) {
    const char *linka = bus["linka"];
    const char *ciel = bus["cielStr"];
    const char *odjazd = bus["odjazd"];
    Serial.printf("%s %s %s\n", linka, ciel, odjazd);
  };

  return 0;
}

void mhd_loop() {
  // TODO
}