#ifndef MHD_H
#define MHD_H

#include <HTTPClient.h>

#define MHD_URL "https://imhd.sk/rt/sio2/"

int mhd_get();
int mhd_connect();
void mhd_loop();

#endif