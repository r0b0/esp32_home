#ifndef SOCKETIO_H
#define SOCKETIO_H

#include <Arduino.h>
#include <HTTPClient.h>

#define ENGINEIO_SEPARATOR_CHAR 0x1e

#define ENGINEIO_PTYPE_OPEN    '0'
#define ENGINEIO_PTYPE_CLOSE   '1'
#define ENGINEIO_PTYPE_PING    '2'
#define ENGINEIO_PTYPE_PONG    '3'
#define ENGINEIO_PTYPE_MESSAGE '4'
#define ENGINEIO_PTYPE_UPGRADE '5'
#define ENGINEIO_PTYPE_NOOP    '6'

#define SOCKETIO_PTYPE_CONNECT       '0'
#define SOCKETIO_PTYPE_DISCONNECT    '1'
#define SOCKETIO_PTYPE_EVENT         '2'
#define SOCKETIO_PTYPE_ACK           '3'
#define SOCKETIO_PTYPE_CONNECT_ERROR '4'
#define SOCKETIO_PTYPE_BINARY_EVENT  '5'
#define SOCKETIO_PTYPE_BINARY_ACK    '6'

typedef void (*handle_string_f)(String);

class SocketIo {
  handle_string_f event_callback;
  String url;
  String sid;
  HTTPClient http;
  int ping_timeout;
  int next_rec_millis;
  void process_data(String data);
  int get_packet_count(String data);
  int send(char engine_io_type, char socket_io_type, String payload);
  String get_packet(String data, int index);
  public:
    SocketIo(String url);
    void set_event_callback(handle_string_f);
    int connect(String);
    int send_event(String);
    int receive();
    void loop();
};



#endif