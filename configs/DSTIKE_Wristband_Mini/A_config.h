#ifndef config_h
#define config_h

// ===== DISPLAY LIBRARY ===== //
#include "SH1106Wire.h"
// =========================== //

// ===================== LED CONFIG ==================== //
#define NEOPIXEL_LED
#define HIGHLIGHT_LED 16

#define LED_MODE_BRIGHTNESS 10 // brightness of LED modes 

#define LED_NEOPIXEL_NUM 1
#define LED_NEOPIXEL_PIN 15
#define LED_NEOPIXEL_MODE NEO_GRBW + NEO_KHZ800

// ===================== DISPLAY CONFIG ==================== //
#define USE_DISPLAY true // default display setting

#define BUTTON_UP 12
#define BUTTON_DOWN 13
#define BUTTON_A 14

#define DEAUTHER_DISPLAY SH1106Wire display = SH1106Wire(0x3c, 5, 4); // for 1.3" OLED
// ========================================================= //

#endif