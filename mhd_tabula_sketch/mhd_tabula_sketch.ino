#include <WiFi.h>
#include "wifi_creds.h"
#include "mhd.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() < startAttempt + 10000) {
    delay(500);
  }
  Serial.printf("wifi status: %s\n",
    WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");

  int connected = mhd_connect();
  Serial.printf("Connected: %d\n", connected);
  // put your setup code here, to run once:

}

void loop() {
  // Serial.println("Hello World");
  mhd_loop();
  delay(2000);

  // put your main code here, to run repeatedly:

}
