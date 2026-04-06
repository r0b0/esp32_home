#include <lvgl.h>

#include "gui.h"

lv_obj_t *gui_make_btn(lv_obj_t *parent, const char *text, lv_palette_t color) {
  lv_obj_t *b = lv_button_create(parent);
  lv_obj_set_style_bg_color(b, lv_palette_main(color), 0);
  lv_obj_remove_flag(b, LV_OBJ_FLAG_PRESS_LOCK);
  lv_obj_t *label = lv_label_create(b);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  return b;
}

static void event_handler_back_btn(lv_event_t *e) {
  lv_screen_load_anim(app.main_screen, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 200, 100, false);
}

static void event_handler_screen_load_btn(lv_event_t * e) {
  lv_screen_load_anim((lv_obj_t *)e->user_data, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 200, 100, false);
}

struct AppScreen *gui_make_screen(const char *name, lv_palette_t color) {
  struct AppScreen *screen = (struct AppScreen *)malloc(sizeof(struct AppScreen));

  screen->screen = lv_obj_create(NULL);
  lv_obj_set_layout(screen->screen, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(screen->screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(screen->screen, LV_PCT(100), LV_PCT(100));

  screen->top_flex = lv_obj_create(screen->screen);
  lv_obj_set_style_pad_ver(screen->top_flex, 0, 0);
  lv_obj_set_size(screen->top_flex, LV_PCT(100), LV_SIZE_CONTENT);

  lv_obj_t *back_btn = gui_make_btn(screen->top_flex, LV_SYMBOL_LEFT " Back", LV_PALETTE_TEAL);
  lv_obj_add_event_cb(back_btn, event_handler_back_btn, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(screen->top_flex);
  lv_label_set_text(label, name);
  lv_obj_align_to(label, back_btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_obj_set_flex_grow(label, 1);

  screen->main_flex = lv_obj_create(screen->screen);
  lv_obj_set_width(screen->main_flex, LV_PCT(100));
  lv_obj_set_flex_grow(screen->main_flex, 1);

  lv_obj_t *main_scr_btn = gui_make_btn(app.main_screen, name, color);
  lv_obj_add_event_cb(main_scr_btn, event_handler_screen_load_btn, LV_EVENT_CLICKED, screen->screen);

  return screen;
}
