#ifndef GUI_H
#define GUI_H

#include "app.h"
extern struct AppStruct app;
lv_obj_t *gui_make_btn(lv_obj_t *parent, const char *text, lv_palette_t color);
struct AppScreen *gui_make_screen(const char *name);

#endif