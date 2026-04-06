#ifndef HW_H
#define HW_H

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

// LCD pin
#define LCD_BACK_LIGHT_PIN 21

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data);
void setup_hw();

void turn_off();
void turn_on();

#endif