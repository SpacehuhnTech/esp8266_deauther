#include "LEDController.h"

LEDController::LEDController() {}

LEDController::~LEDController() {
    if (led) delete led;
}

void LEDController::setup() {
  #ifdef DIGITAL_LED
    led = new DigitalLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_ANODE);
    led->setup();
    return;

  #endif // ifdef DIGITAL_LED

  #ifdef RGB_LED
    led = new LEDController::AnalogRGBLED(LED_PIN_R, LED_PIN_G, LED_PIN_B, LED_MODE_BRIGHTNESS, LED_ANODE);
    led->setup();
    return;

  #endif // ifdef RGB_LED

  #ifdef NEOPIXEL_LED
    led = new LEDController::NeopixelLED(LED_NEOPIXEL_NUM, LED_NEOPIXEL_PIN, LED_MODE_BRIGHTNESS);
    led->setup();
    return;

  #endif // ifdef NEOPIXEL_LED

    prntln(L_NOT_CONFIGURED);
}

void LEDController::update() {
    if (!tempEnabled || !led) return;

    if (!settings.getLedEnabled() && tempEnabled) tempDisable();

    if (scan.isScanning() && (scan.deauths < settings.getMinDeauths())) setMode(LED_MODE::SCAN, false);
    else if (scan.deauths >= settings.getMinDeauths()) setMode(LED_MODE::DEAUTH, false);
    else if (attack.isRunning()) setMode(LED_MODE::ATTACK, false);
    else setMode(LED_MODE::IDLE, false);
}

void LEDController::setMode(uint8_t mode, bool force) {
    if (!led) return;

    if ((mode != LEDController::mode) || force) {
        LEDController::mode = mode;

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

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b, bool output) {
    // debug output
    if (output) {
        char s[30];
        sprintf_P(s, L_OUTPUT, r, g, b);
        prnt(String(s));
    }

    led->setColor(r, g, b);
}

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output) {
    led->setBrightness(brightness);
    setColor(r, g, b, output);
}

void LEDController::tempEnable() {
    tempEnabled = true;
    prntln(L_ENABLED);
}

void LEDController::tempDisable() {
    tempEnabled = false;
    prntln(L_DISABLED);
}

bool LEDController::getTempEnabled() {
    return tempEnabled;
}

// ===== DigitalLED ===== //
LEDController::DigitalLED::DigitalLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, bool anode) {
    LEDController::DigitalLED::anode = anode;
    LEDController::DigitalLED::rPin  = rPin;
    LEDController::DigitalLED::gPin  = gPin;
    LEDController::DigitalLED::bPin  = bPin;
}

LEDController::DigitalLED::~DigitalLED() {}

void LEDController::DigitalLED::setup() {
    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void LEDController::DigitalLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
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

void LEDController::DigitalLED::setBrightness(uint8_t brightness) {}


// ===== AnalogRGBLED ===== //
LEDController::AnalogRGBLED::AnalogRGBLED(uint8_t rPin, uint8_t gPin, uint8_t bPin, uint8_t brightness, bool anode) {
    LEDController::AnalogRGBLED::anode = anode;
    LEDController::AnalogRGBLED::rPin  = rPin;
    LEDController::AnalogRGBLED::gPin  = gPin;
    LEDController::AnalogRGBLED::bPin  = bPin;
    setBrightness(brightness);
}

LEDController::AnalogRGBLED::~AnalogRGBLED() {}

void LEDController::AnalogRGBLED::setup() {
    analogWriteRange(0xff);

    if (rPin < 255) pinMode(rPin, OUTPUT);

    if (gPin < 255) pinMode(gPin, OUTPUT);

    if (bPin < 255) pinMode(bPin, OUTPUT);
}

void LEDController::AnalogRGBLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
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

void LEDController::AnalogRGBLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    LEDController::AnalogRGBLED::brightness = brightness;
}

// ===== NeopixelLED ===== //
LEDController::NeopixelLED::NeopixelLED(int num, uint8_t dataPin, uint8_t brightness) {
    strip = new Adafruit_NeoPixel(num, dataPin, LED_NEOPIXEL_MODE);
    setBrightness(brightness);
}

LEDController::NeopixelLED::~NeopixelLED() {
    delete strip;
}

void LEDController::NeopixelLED::setup() {
    strip->begin();
    strip->show();
}

void LEDController::NeopixelLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    int num = strip->numPixels();

    for (uint16_t i = 0; i < num; i++) strip->setPixelColor(i, strip->Color(r, g, b));
    strip->show();
}

void LEDController::NeopixelLED::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    strip->setBrightness(brightness);
}