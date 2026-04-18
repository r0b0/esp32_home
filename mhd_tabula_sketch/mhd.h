#ifndef MHD_H
#define MHD_H

#include <HTTPClient.h>

#define MHD_URL "http://192.168.1.15:3000/rt/sio2/"

int mhd_get();
int mhd_connect();
void mhd_loop();

#endif