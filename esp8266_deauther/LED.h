#ifndef LED_h
#define LED_h

// ===== [Includes] ===== //
#include "Arduino.h"  // digitalWrite, analogWrite, pinMode
#include "A_config.h" // Config for LEDs
#include "language.h" // Strings used in printColor and tempDisable

// ===== [Defines] ===== //

// Inlcude libraries for Neopixel or LED_MY92xx if used
#if defined(NEOPIXEL_LED)
#include <Adafruit_NeoPixel.h>
#elif defined(LED_MY92)
#include <my92xx.h>
#endif // if defined(NEOPIXEL_LED)

// ===== [Strings] ===== //
const char L_OUTPUT[] PROGMEM = "LED = (%u,%u,%u)";

// ===== [LED Mode Enum] ===== //
enum LED_MODE {
    OFF,
    SCAN,
    ATTACK,
    IDLE
};

// ===== [LED Class] ===== //
class LED {
    private:
        LED_MODE mode = OFF;

#if defined(LED_NEOPIXEL_RGB)
        Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, NEO_RGB + NEO_KHZ400);
#elif defined(LED_NEOPIXEL_GRB)
        Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ400);
#elif defined(LED_MY92)
        my92xx myled = my92xx(LED_MY92_MODEL, LED_MY92_NUM, LED_MY92_DATA, LED_MY92_CLK, MY92XX_COMMAND_DEFAULT);
#endif // if defined(NEOPIXEL_LED)

        void setColor(uint8_t r, uint8_t g, uint8_t b, bool output = false);

    public:
        void setup();
        void update();
        void setMode(LED_MODE mode, bool force = false);
};

#endif // ifndef LED_h