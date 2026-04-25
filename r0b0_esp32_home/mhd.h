#ifndef MHD_H
#define MHD_H

#include <HTTPClient.h>

// #define MHD_URL "http://aspire.lamac.cc:3000/rt/sio2/"
#define MHD_URL "https://imhd.sk/rt/sio2/"

typedef void (*handle_bus_f)(String linka, String ciel, String odjazd, bool is_first, bool is_last);

int mhd_connect(int stop, String platform, handle_bus_f callback);
void mhd_loop();

#endif