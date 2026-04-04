#include <Arduino.h>

char *copy_string(String s) {
    char *c = (char *)malloc(s.length());
    strcpy(c, s.c_str());
    return c;
}
