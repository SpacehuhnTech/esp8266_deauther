#ifndef LED_h
#define LED_h

#include "Arduino.h"  // digitalWrite, analogWrite, pinMode
#include "A_config.h" // Config for LEDs

// Inlcude libraries for Neopixel or LED_MY92xx if used
#if defined(NEOPIXEL_LED)
#include <Adafruit_NeoPixel.h>
#elif defined(LED_MY92)
#include <my92xx.h>
#endif // if defined(NEOPIXEL_LED)

enum LED_MODE {
    OFF    = 0,
    SCAN   = 1,
    ATTACK = 2,
    DEAUTH = 3,
    IDLE   = 4
};

class LED {
    private:
        bool tempEnabled = true;
        LED_MODE mode    = OFF;

        uint8_t brightness = 100;

#if defined(LED_NEOPIXEL_RGB)
        Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, NEO_RGB + NEO_KHZ400);
#elif defined(LED_NEOPIXEL_GRB)
        Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ400);
#elif defined(LED_MY92)
        my92xx myled = my92xx(LED_MY92_MODEL, LED_MY92_NUM, LED_MY92_DATA, LED_MY92_CLK, MY92XX_COMMAND_DEFAULT);
#endif // if defined(NEOPIXEL_LED)

    public:
        void setup() {
            analogWriteRange(0xff);

            brightness = LED_MODE_BRIGHTNESS;

#if defined(LED_DIGITAL) || defined(LED_RGB)
            if (LED_PIN_R < 255) pinMode(LED_PIN_R, OUTPUT);
            if (LED_PIN_G < 255) pinMode(LED_PIN_G, OUTPUT);
            if (LED_PIN_B < 255) pinMode(LED_PIN_B, OUTPUT);
#elif defined(NEOPIXEL_LED)
            strip.begin();
            strip.setBrightness(brightness);
            strip.show();
#elif defined(LED_MY9291)
            myled.setChannel(LED_MY92_CH_R, 0);
            myled.setChannel(LED_MY92_CH_G, 0);
            myled.setChannel(LED_MY92_CH_B, 0);
            myled.setChannel(LED_MY92_CH_BRIGHTNESS, brightness);
            myled.setState(true);
            myled.update();
#endif // if defined(LED_DIGITAL) || defined(LED_RGB)
        }

        void update();

        void printColor(uint8_t r, uint8_t g, uint8_t b);

        void setMode(LED_MODE mode, bool force = false);
        void setBrightness(uint8_t brightness);

        void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output = false) {
            setBrightness(brightness);
            setColor(r, g, b, output);
        }

        void setColor(uint8_t r, uint8_t g, uint8_t b, bool output = false) {
            if (output) printColor(r, g, b);

#if defined(LED_DIGITAL)
            if (LED_ANODE) {
                if (LED_PIN_R < 255) digitalWrite(LED_PIN_R, r > 0);
                if (LED_PIN_G < 255) digitalWrite(LED_PIN_G, g > 0);
                if (LED_PIN_B < 255) digitalWrite(LED_PIN_B, b > 0);
            } else {
                if (LED_PIN_R < 255) digitalWrite(LED_PIN_R, r == 0);
                if (LED_PIN_G < 255) digitalWrite(LED_PIN_G, g == 0);
                if (LED_PIN_B < 255) digitalWrite(LED_PIN_B, b == 0);
            }
#elif defined(LED_RGB)
            if ((r > 0) && (brightness > 0)) r = r * brightness / 100;
            if ((g > 0) && (brightness > 0)) g = g * brightness / 100;
            if ((b > 0) && (brightness > 0)) b = b * brightness / 100;

            if (LED_ANODE) {
                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
            }

            analogWrite(LED_PIN_R, r);
            analogWrite(LED_PIN_G, g);
            analogWrite(LED_PIN_B, b);
#elif defined(NEOPIXEL_LED)

            for (size_t i = 0; i < LED_NEOPIXEL_NUM; i++) {
                strip.setPixelColor(i, r, g, b);
            }

            strip.show();
#elif defined(LED_MY9291)
            myled.setChannel(LED_MY92_CH_R, r);
            myled.setChannel(LED_MY92_CH_G, g);
            myled.setChannel(LED_MY92_CH_B, b);
            myled.setChannel(LED_MY92_CH_BRIGHTNESS, brightness);
            myled.setState(true);
            myled.update();
#endif // if defined(LED_DIGITAL)
        }

        void tempEnable();
        void tempDisable();
        bool getTempEnabled();
};

#endif // ifndef LED_h