#include <Arduino.h>

char *copyString(String s) {
    char *c = (char *)malloc(s.length());
    strcpy(c, s.c_str());
    return c;
}
