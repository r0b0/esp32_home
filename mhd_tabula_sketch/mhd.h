#ifndef MHD_H
#define MHD_H

#include <HTTPClient.h>

#ifdef MHD_PROD
#define MHD_URL "https://imhd.sk/rt/sio2/"
#else
#define MHD_URL "http://aspire.lamac.cc:3000/rt/sio2/"
#endif

int mhd_get();
int mhd_connect();
void mhd_loop();

#endif