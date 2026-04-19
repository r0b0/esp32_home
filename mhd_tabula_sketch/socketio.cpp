#include <Arduino.h>
#include <ArduinoJson.h>
#include "socketio.h"

handle_string_f socketio_sid_callback;
handle_string_f socketio_message_callback;

int socketio_get_packet_count(String data) {
  int count = 1;
  for(int i=0; i<data.length(); i++) {
    if(data.charAt(i) == ENGINEIO_SEPARATOR_CHAR)
      count++;
  }
  return count;
}

String socketio_get_packet(String data, int index) {
  int packet_end = data.indexOf(ENGINEIO_SEPARATOR_CHAR);
  if(!index) {
    if(packet_end<0) {
      return data;
    } else {
      return data.substring(0,packet_end);
    }
  } else {
    String next = data.substring(packet_end+1);
    return socketio_get_packet(next, index-1);
  }
}

void socketio_process_data(String data) {
  for(int i=0; i<socketio_get_packet_count(data); i++) {
    String packet = socketio_get_packet(data, i);
    String message_data;
    char socketio_packet_type;
    JsonDocument doc;
    DeserializationError json_error;
    String sid;
    char engineio_packet_type = packet[0];
    switch(engineio_packet_type) {
      case ENGINEIO_PTYPE_OPEN:
        message_data = packet.substring(1);
        Serial.printf("Packet #%d engine.io type OPEN data %s\n",
          i, message_data.c_str());
        json_error = deserializeJson(doc, message_data);
        if(json_error) {
          Serial.println(json_error.c_str());
          break;
        }
        sid = doc["sid"].as<String>();
        Serial.printf("  new sid %s\n", sid.c_str());
        if(socketio_sid_callback) 
          socketio_sid_callback(sid);
        break;
      case ENGINEIO_PTYPE_MESSAGE:
        socketio_packet_type = packet[1];
        message_data = packet.substring(2);
        Serial.printf("Packet #%d engine.io type MESSAGE socket.io type %c data %s\n",
          i, socketio_packet_type, message_data.c_str());
        if(socketio_packet_type == SOCKETIO_PTYPE_EVENT && socketio_message_callback)
          socketio_message_callback(message_data);
        break;
      default:
        message_data = packet.substring(1);
        Serial.printf("Packet #%d engine.io type %c data %s\n",
          i, engineio_packet_type, message_data.c_str());
    }
  }
}

void socketio_set_sid_callback(handle_string_f f) {
  socketio_sid_callback = f;
}

void socketio_set_message_callback(handle_string_f f) {
  socketio_message_callback = f;
}