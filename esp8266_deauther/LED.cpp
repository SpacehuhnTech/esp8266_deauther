#include "LED.h"

LED::LED() {}

LED::~LED() {
    if (led) delete led;
}

void LED::setup() {
  #if defined(DIGITAL_LED)
    led = new DigitalLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_ANODE);
    led->setup();
  #elif defined(RGB_LED)
    led = new LED::AnalogRGBLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_MODE_BRIGHTNESS, LED_ANODE);
    led->setup();
  #elif defined(NEOPIXEL_LED)
    led = new LED::NeopixelLED(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, LED_MODE_BRIGHTNESS);
    led->setup();
  #endif // if defined(DIGITAL_LED)
}

void LED::update() {
    if (!tempEnabled || !led) return;

    if (!settings.getLedEnabled() && tempEnabled) tempDisable();

    if (scan.isScanning() && (scan.deauths < settings.getMinDeauths())) setMode(LED_MODE::SCAN, false);
    else if (scan.deauths >= settings.getMinDeauths()) setMode(LED_MODE::DEAUTH, false);
    else if (attack.isRunning()) setMode(LED_MODE::ATTACK, false);
    else setMode(LED_MODE::IDLE, false);
}

void LED::setMode(uint8_t mode, bool force) {
    if (!led) return;

    if ((mode != LED::mode) || force) {
        LED::mode = mode;

        switch (mode) {
        case LED_MODE::OFF:
            led->setColor(0, 0, 0);
            break;

        case LED_MODE::SCAN:
            led->setColor(0, 0, 255);
            break;

        case LED_MODE::ATTACK:
            led->setColor(255, 255, 0);
            break;

        case LED_MODE::DEAUTH:
            led->setColor(255, 0, 0);
            break;

        case LED_MODE::IDLE:
            led->setColor(0, 255, 0);
            break;
        }
    }
}

void LED::setColor(uint8_t r, uint8_t g, uint8_t b, bool output) {
    // debug output
    if (output) {
        char s[30];
        sprintf_P(s, L_OUTPUT, r, g, b);
        prnt(String(s));
    }

    led->setColor(r, g, b);
}

void LED::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output) {
    led->setBrightness(brightness);
    setColor(r, g, b, output);
}

void LED::tempEnable() {
    tempEnabled = true;
    prntln(L_ENABLED);
}

void LED::tempDisable() {
    tempEnabled = false;
    prntln(L_DISABLED);
}

bool LED::getTempEnabled() {
    return tempEnabled;
}

#ifdef DIGITAL_LED
// ===== DigitalLED ===== //
LED::DigitalLED::DigitalLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, bool anode) {
    LED::DigitalLED::anode = anode;
    LED::DigitalLED::rPin  = rPin;
    LED::DigitalLED::gPin  = gPin;
    LED::DigitalLED::bPin  = bPin;
}

LED::DigitalLED::~DigitalLED() {}

void LED::DigitalLED::setup() {
    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void LED::DigitalLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (anode) {
        if (rPin < 255) digitalWrite(rPin, r > 0);

        if (gPin < 255) digitalWrite(gPin, g > 0);

        if (bPin < 255) digitalWrite(bPin, b > 0);
    } else {
        if (rPin < 255) digitalWrite(rPin, r == 0);

        if (gPin < 255) digitalWrite(gPin, g == 0);

        if (bPin < 255) digitalWrite(bPin, b == 0);
    }
}

void LED::DigitalLED::setBrightness(uint8_t brightness) {}
#endif

#ifdef RGB_LED
// ===== AnalogRGBLED ===== //
LED::AnalogRGBLED::AnalogRGBLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, uint8_t brightness, bool anode) {
    LED::AnalogRGBLED::anode = anode;
    LED::AnalogRGBLED::rPin  = rPin;
    LED::AnalogRGBLED::gPin  = gPin;
    LED::AnalogRGBLED::bPin  = bPin;

    setBrightness(brightness);
}

LED::AnalogRGBLED::~AnalogRGBLED() {}

void LED::AnalogRGBLED::setup() {
    analogWriteRange(0xff);

    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void LED::AnalogRGBLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if ((r > 0) && (brightness < 100)) r = r * brightness / 100;

    if ((g > 0) && (brightness < 100)) g = g * brightness / 100;

    if ((b > 0) && (brightness < 100)) b = b * brightness / 100;

    if (anode) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }

    analogWrite(rPin, r);
    analogWrite(gPin, g);
    analogWrite(bPin, b);
}

void LED::AnalogRGBLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    LED::AnalogRGBLED::brightness = brightness;
}
#endif

#ifdef NEOPIXEL_LED
// ===== NeopixelLED ===== //
LED::NeopixelLED::NeopixelLED(int num, uint8_t dataPin, uint8_t brightness) {
    strip = new Adafruit_NeoPixel(num, dataPin, LED_NEOPIXEL_MODE);
    setBrightness(brightness);
}

LED::NeopixelLED::~NeopixelLED() {
    delete strip;
}

void LED::NeopixelLED::setup() {
    strip->begin();
    strip->show();
}

void LED::NeopixelLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    int num = strip->numPixels();

    for (uint16_t i = 0; i < num; i++) strip->setPixelColor(i, strip->Color(r, g, b));
    strip->show();
}

void LED::NeopixelLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    strip->setBrightness(brightness);
}
#endif
