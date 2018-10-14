#ifndef config_h
#define config_h

// ===== DISPLAY LIBRARY ===== //
#include <Wire.h>
#include <SPI.h>
#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "SH1106Spi.h"
#include "SSD1306Spi.h"
// =========================== //

//#define DEFAULT_SSID "pwned"
//#define HIGHLIGHT_LED 16

// ===================== LED CONFIG ==================== //
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Uncomment the type of LED  you're using
    Only one of them can be defined at a time!
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#define DIGITAL_LED
//#define RGB_LED
//#define NEOPIXEL_LED

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Adjust following settings for your type of LED
    you can ignore the rest of the #define's
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
   
// === Settings for Digital LED and RGB LED === //
#define LED_ANODE false   // common vcc (+)
#define LED_PIN_R 16      // NodeMCU on-board LED
#define LED_PIN_G 255     // 255 = LED disabled
#define LED_PIN_B 2       // ESP-12 LED

// === Settings for RGB LED and Neopixel LED === //
#define LED_MODE_BRIGHTNESS 10 // brightness of LED modes 

// === Settings for Neopixel LED === //
#define LED_NEOPIXEL_NUM 1
#define LED_NEOPIXEL_PIN 15
#define LED_NEOPIXEL_MODE NEO_GRB + NEO_KHZ800
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// ===================== DISPLAY CONFIG ==================== //
#define USE_DISPLAY false // default display setting

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Adjust the pins to match your setup
    Comment out the buttons you don't use.
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#define BUTTON_UP 14
#define BUTTON_DOWN 12
#define BUTTON_A 13

//#define BUTTON_LEFT 12
//#define BUTTON_RIGHT 13
//#define BUTTON_B 10

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Uncomment the type of display you're using
    Only one of them can be defined at a time!
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
*/
#define DEAUTHER_DISPLAY SSD1306Wire display = SSD1306Wire(0x3c, 5, 4); // for 0.96" OLED
//#define DEAUTHER_DISPLAY SH1106Wire display = SH1106Wire(0x3c, 5, 4); // for 1.3" OLED

/*  RST = GPIO 5 (D1)
    DC = GPIO 4 (D2)
    CS = GPIO 15 (D8) or GND
    SCK/SCL = GPIO 14 (D5)
    SDA/MOSI = GPIO 13 (D7) */
//#define DEAUTHER_DISPLAY SSD1306Spi display = SSD1306Spi display = SSD1306Spi(5, 4, 15); // for 0.96" OLED with SPI
//#define DEAUTHER_DISPLAY SH1106Spi display = SH1106Spi(5, 4, 15); // for 1.3" OLED with SPI

//#define FLIP_DIPLAY // uncomment that to flip the display vertically
// ========================================================= //

#endif
