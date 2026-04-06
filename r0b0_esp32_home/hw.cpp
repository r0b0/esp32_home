#include "misc/lv_log.h"
#include "esp32-hal.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "hw.h"
#include "app.h"

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    app.last_touch = millis();
    if(app.turned_on) {
      // Get Touchscreen points
      TS_Point p = touchscreen.getPoint();
      // Calibrate Touchscreen points with map function to the correct width and height
      x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
      y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
      z = p.z;

      data->state = LV_INDEV_STATE_PRESSED;

      // Set the coordinates
      data->point.x = x;
      data->point.y = y;

      // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
      /* Serial.print("X = ");
      Serial.print(x);
      Serial.print(" | Y = ");
      Serial.print(y);
      Serial.print(" | Pressure = ");
      Serial.print(z);
      Serial.println();*/
    } else {
      data->state = LV_INDEV_STATE_RELEASED;  // if turned off, just turn on
      turn_on();
    }
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void setup_hw() {
  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  pinMode(LCD_BACK_LIGHT_PIN, OUTPUT);
}

void turn_off() {
  if(app.turned_on) {
    LV_LOG_USER("Turning off");
    digitalWrite(LCD_BACK_LIGHT_PIN, LOW);  // turn off back-light
    app.turned_on = false;
  }
}

void turn_on() {
  if(!app.turned_on) {
    LV_LOG_USER("Turning on");
    digitalWrite(LCD_BACK_LIGHT_PIN, HIGH);
    app.turned_on = true;
  }
}