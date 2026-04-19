#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "socketio.h"

SocketIo::SocketIo(String u) {
  url = u + String("?EIO=4&transport=polling");
  http.begin(url);
  http.setTimeout(30000);
}

int SocketIo::receive() {
  http.begin(url);
  int ret = http.GET();
  /*
  // UNFORTUNATELY, THIS DOES NOT WORK
  if(ret == HTTPC_ERROR_READ_TIMEOUT) {
    Serial.println("Timeout received on long poll, will re-try on next loop");
    return 0;
  }
  */
  if(ping_timeout) {
    next_rec_millis = millis() + (ping_timeout * 0.8);
  }
  String received_data = http.getString();
  Serial.printf("received data: %d %s\n", ret, received_data.c_str());
  if(ret != 200) {
    return -1;
  }
  process_data(received_data);
  return 0;
}

int SocketIo::send(char engine_io_type, char socket_io_type, String payload) {
  http.begin(url);
  String data = String(engine_io_type);
  if(socket_io_type) {
    data = data + String(socket_io_type);
    if(payload.length()) {
      data = data + payload;
    }
  }
  Serial.printf("POSTing to %s: %s", url.c_str(), data.c_str());
  int ret = http.POST(data);
  if(ret != 200) {
    return -1;
  }
  String resp = http.getString();
  if(!resp.equals("ok")) {
    return -2;
  }
  return 0;
}

int SocketIo::send_event(String payload) {
  return send(ENGINEIO_PTYPE_MESSAGE, SOCKETIO_PTYPE_EVENT, payload);
}

int SocketIo::get_packet_count(String data) {
  int count = 1;
  for(int i=0; i<data.length(); i++) {
    if(data.charAt(i) == ENGINEIO_SEPARATOR_CHAR)
      count++;
  }
  return count;
}

String SocketIo::get_packet(String data, int index) {
  int packet_end = data.indexOf(ENGINEIO_SEPARATOR_CHAR);
  if(!index) {
    if(packet_end<0) {
      return data;
    } else {
      return data.substring(0,packet_end);
    }
  } else {
    String next = data.substring(packet_end+1);
    return get_packet(next, index-1);
  }
}

void SocketIo::process_data(String data) {
  for(int i=0; i<get_packet_count(data); i++) {
    String packet = get_packet(data, i);
    String message_data;
    char socketio_packet_type;
    JsonDocument doc;
    DeserializationError json_error;
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
        url = url + String("&sid=") + sid;
        ping_timeout = doc["pingTimeout"];
        Serial.printf("  new sid %s, ping timeout %d\n", sid.c_str(), ping_timeout);
        break;
      case ENGINEIO_PTYPE_MESSAGE:
        socketio_packet_type = packet[1];
        message_data = packet.substring(2);
        Serial.printf("Packet #%d engine.io type MESSAGE socket.io type %c data %s\n",
          i, socketio_packet_type, message_data.c_str());
        if(socketio_packet_type == SOCKETIO_PTYPE_EVENT && event_callback)
          event_callback(message_data);
        break;
      case ENGINEIO_PTYPE_PING:
        Serial.printf("Packet #%d engine.io type PING, sending a PONG\n", i);
        send(ENGINEIO_PTYPE_PONG, 0, "");
        break;
      default:
        message_data = packet.substring(1);
        Serial.printf("Packet #%d engine.io type %c data %s\n",
          i, engineio_packet_type, message_data.c_str());
    }
  }
}

void SocketIo::set_event_callback(handle_string_f f) {
  event_callback = f;
}

int SocketIo::connect(String nmspc) {
  int err;
  err = receive();
  if(err) {
    Serial.printf("Socket connect failed: %d\n", err);
    return err;
  } 

  err = send(ENGINEIO_PTYPE_MESSAGE, SOCKETIO_PTYPE_CONNECT, nmspc);
  if(err) {
    Serial.printf("namespace connect failed: %d\n", err);
    return err;
  }

  err = receive();
  if(err) {
    Serial.printf("Socket receive failed: %d\n", err);
    return err;
  }

  return 0;
}

void SocketIo::loop() {
  if(next_rec_millis > 0 && millis() > next_rec_millis) {
    Serial.println("Fetching data from the loop");
    receive();
  }
}