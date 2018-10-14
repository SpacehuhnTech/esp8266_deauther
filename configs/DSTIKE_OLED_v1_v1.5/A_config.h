#ifndef config_h
#define config_h

// ===== DISPLAY LIBRARY ===== //
#include "SSD1306Wire.h"
// =========================== //

// ===================== LED CONFIG ==================== //
#define DIGITAL_LED
#define LED_ANODE false   // common vcc (+)
#define LED_PIN_R 16      // NodeMCU on-board LED
#define LED_PIN_G 255     // 255 = LED disabled
#define LED_PIN_B 2       // ESP-12 LED

// ===================== DISPLAY CONFIG ==================== //
#define USE_DISPLAY true // default display setting

#define BUTTON_UP 12
#define BUTTON_DOWN 13
#define BUTTON_A 14

#define DEAUTHER_DISPLAY SSD1306Wire display = SSD1306Wire(0x3c, 5, 4); // for 0.96" OLED
// ========================================================= //

#endif