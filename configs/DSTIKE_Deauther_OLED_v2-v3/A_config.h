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
// === Settings for Digital LED === //
#define LED_ENABLE_R true
#define LED_ENABLE_G false
#define LED_ENABLE_B true

// === Settings for Digital LED and RGB LED === //
#define LED_CATHODE false // common ground (GND)
#define LED_PIN_R 16      // NodeMCU on-board LED
#define LED_PIN_G 0
#define LED_PIN_B 2       // ESP-12 LED

// === Settings for RGB LED and Neopixel LED === //
#define LED_MODE_BRIGHTNESS 10 // brightness of LED modes 
#define LED_DYNAMIC_BRIGHTNESS false // brightness in scan mode depending on packet rate and deauths per second

// === Settings for Neopixel LED === //
#define LED_NEOPIXEL_NUM 1
#define LED_NEOPIXEL_PIN 9
// if it doesn't work try changing NEO_GRB to NEO_GRBW 
#define LED_NEOPIXEL Adafruit_NeoPixel(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)

// ===================== DISPLAY CONFIG ==================== //
#define USE_DISPLAY true // default display setting

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Adjust the pins to match your setup
    Comment out the buttons you don't use.
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#define BUTTON_UP 12
#define BUTTON_DOWN 13
#define BUTTON_A 14

//#define BUTTON_LEFT 12
//#define BUTTON_RIGHT 13
//#define BUTTON_B 10

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    Uncomment the type of display you're using
    Only one of them can be defined at a time!
   = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    0x3c = I2C address
    SDA = GPIO 5 (D1)  
    SCL/SCK = GPIO 4 (D2) */
//#define DEAUTHER_DISPLAY SSD1306Wire display = SSD1306Wire(0x3c, 5, 4); // for 0.96" OLED
#define DEAUTHER_DISPLAY SH1106Wire display = SH1106Wire(0x3c, 5, 4); // for 1.3" OLED

/*  RST = GPIO 5 (D1)
    DC = GPIO 4 (D2)
    CS = GPIO 15 (D8) or GND
    SCK/SCL = GPIO 14 (D5)
    SDA/MOSI = GPIO 13 (D7) */
//#define DISPLAY SSD1306Spi display = SSD1306Spi display = SSD1306Spi(5, 4, 15); // for 0.96" OLED with SPI
//#define DISPLAY SH1106Spi display = SH1106Spi(5, 4); // for 1.3" OLED with SPI

//#define FLIP_DIPLAY // uncomment that to flip the display vertically
// ========================================================= //

#endif