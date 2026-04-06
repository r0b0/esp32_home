#include "weather.h"
#include "app.h"
#include <Arduino.h>
#include <lvgl.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetch_weather() {
  HTTPClient http;
  http.begin(WEATHER_URL);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String weather = http.getString();
    LV_LOG_USER("Fetched weather %d B", weather.length());
    JsonDocument weatherJson;
    deserializeJson(weatherJson, weather);
    JsonObject data = weatherJson["data"];
    LV_LOG_USER("Deserialized weather %d objects", data.size());

    String city = String(data["request"][0]["query"]);
    String time = String(data["time_zone"][0]["localtime"]);
    
    JsonObject current_condition = data["current_condition"][0];
    String current_temp = String(current_condition["temp_C"]);
    String current_weather_desc = String(current_condition["weatherDesc"][0]["value"]);
    String current_wind_speed = String(current_condition["windspeedKmph"]);
    String current_precip = String(current_condition["precipMM"]);
    String weather_string = city + " " + time + "\n" + 
      current_temp + "C " + current_weather_desc + " Wind: " + current_wind_speed + "km/h Prec: " + current_precip + "mm";

    for (JsonObject data_weather_item : data["weather"].as<JsonArray>()) {
      String date = String(data_weather_item["date"]);
      // LV_LOG_USER(" date %s", date.c_str());
      String max_temp = String(data_weather_item["maxtempC"]);
      String min_temp = String(data_weather_item["mintempC"]);
      weather_string = weather_string + "\n" + date + " " + min_temp + "-" + max_temp + "C ";
      
      for (JsonObject hourly : data_weather_item["hourly"].as<JsonArray>()) {
        String time = String(hourly["time"]);
        // LV_LOG_USER(" time %s", time.c_str());
        if(time != "1200")
          continue;
        // String time_temp = String(hourly["tempC"]);
        String time_weather_desc = String(hourly["weatherDesc"][0]["value"]);
        String chanceofrain = String(hourly["chanceofrain"]);
        weather_string = weather_string + time_weather_desc + " " + chanceofrain + "%rain";
      }
    }

    for (JsonObject alert : data["alerts"]["alert"].as<JsonArray>()) {
      String area = String(alert["areas"]);
      if(area != "Bratislava")
        continue;

      String severity = String(alert["severity"]);
      String event = String(alert["event"]);
      weather_string = weather_string + "\n" + severity + " alert: " + event;
    }
    
    lv_label_set_text(app.weather_label, weather_string.c_str());
  } else {
    LV_LOG_USER("http call failed %d", httpCode);
  }
}

void event_handler_weather_refresh(lv_event_t *e) {
  fetch_weather();
}

