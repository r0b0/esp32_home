#ifndef SOCKETIO_H
#define SOCKETIO_H

#include <Arduino.h>

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

int socketio_get_packet_count(String data);
String socketio_get_packet(String data, int index);
void socketio_process_data(String data);

typedef void (*handle_string_f)(String sid);
void socketio_set_sid_callback(handle_string_f);
void socketio_set_message_callback(handle_string_f);

#endif