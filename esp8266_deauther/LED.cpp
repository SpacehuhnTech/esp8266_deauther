#include "LED.h"

// ===== [Includes] ===== //
// used for update()
#include "Settings.h"
#include "Attack.h"
#include "Scan.h"

// ===== [External] ===== //
// used for update()
extern Settings settings;
extern Attack   attack;
extern Scan     scan;

void LED::setColor(uint8_t r, uint8_t g, uint8_t b, bool output) {
    if (output) {
        char s[30];

        sprintf_P(s, L_OUTPUT, r, g, b);
        prnt(String(s));
    }

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
    if (r > 0) r = r * LED_MODE_BRIGHTNESS / 100;
    if (g > 0) g = g * LED_MODE_BRIGHTNESS / 100;
    if (b > 0) b = b * LED_MODE_BRIGHTNESS / 100;

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
    myled.setChannel(LED_MY92_CH_BRIGHTNESS, LED_MODE_BRIGHTNESS);
    myled.setState(true);
    myled.update();
#endif // if defined(LED_DIGITAL)
}

void LED::setup() {
    analogWriteRange(0xff);

#if defined(LED_DIGITAL) || defined(LED_RGB)
    if (LED_PIN_R < 255) pinMode(LED_PIN_R, OUTPUT);
    if (LED_PIN_G < 255) pinMode(LED_PIN_G, OUTPUT);
    if (LED_PIN_B < 255) pinMode(LED_PIN_B, OUTPUT);
#elif defined(NEOPIXEL_LED)
    strip.begin();
    strip.setBrightness(LED_MODE_BRIGHTNESS);
    strip.show();
#elif defined(LED_MY9291)
    myled.setChannel(LED_MY92_CH_R, 0);
    myled.setChannel(LED_MY92_CH_G, 0);
    myled.setChannel(LED_MY92_CH_B, 0);
    myled.setChannel(LED_MY92_CH_BRIGHTNESS, LED_MODE_BRIGHTNESS);
    myled.setState(true);
    myled.update();
#endif // if defined(LED_DIGITAL) || defined(LED_RGB)
}

void LED::update() {
    if (!settings.getLEDSettings().enabled) {
        setMode(OFF);
    } else if (scan.isScanning() && (scan.deauths < settings.getSnifferSettings().min_deauth_frames)) {
        setMode(SCAN);
    } else if (attack.isRunning()) {
        setMode(ATTACK);
    } else {
        setMode(IDLE);
    }
}

void LED::setMode(LED_MODE mode, bool force) {
    if ((mode != this->mode) || force) {
        this->mode = mode;

        switch (mode) {
            case OFF:
                setColor(LED_MODE_OFF);
                break;

            case SCAN:
                setColor(LED_MODE_SCAN);
                break;

            case ATTACK:
                setColor(LED_MODE_ATTACK);
                break;

            case IDLE:
                setColor(LED_MODE_IDLE);
                break;
        }
    }
}