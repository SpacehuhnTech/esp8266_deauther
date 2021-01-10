#include "AnalogStick.h"

namespace simplebutton {
    AnalogStick::AnalogStick() {
        setup(255, 255, 255);
    }

    AnalogStick::AnalogStick(uint8_t xPin, uint8_t yPin, uint8_t buttonPin) {
        setup(xPin, yPin, buttonPin);
    }

    AnalogStick::~AnalogStick() {
        if (this->up) delete this->up;
        if (this->down) delete this->down;
        if (this->left) delete this->left;
        if (this->right) delete this->right;
    }

    void AnalogStick::setup(uint8_t xPin, uint8_t yPin, uint8_t buttonPin) {
        this->xPin      = xPin;
        this->yPin      = yPin;
        this->buttonPin = buttonPin;

        if (xPin < 255) pinMode(xPin, INPUT);
        if (yPin < 255) pinMode(yPin, INPUT);
        if (buttonPin < 255) pinMode(buttonPin, INPUT);

        this->button = new ButtonPullup(buttonPin);
        this->up     = new ButtonAnalog(yPin);
        this->down   = new ButtonAnalog(yPin);
        this->left   = new ButtonAnalog(xPin);
        this->right  = new ButtonAnalog(xPin);

        setLogic(1024);
    }

    void AnalogStick::update() {
        button->update();
        up->update();
        down->update();
        left->update();
        right->update();

        this->xValue = left->getState();
        this->yValue = up->getState();
    }

    void AnalogStick::update(uint8_t xValue, uint8_t yValue, bool buttonPress) {
        this->xValue = xValue;
        this->yValue = yValue;

        button->update(buttonPress);
        up->update(yValue);
        down->update(yValue);
        left->update(xValue);
        right->update(xValue);
    }

    uint8_t AnalogStick::getX() {
        return xValue;
    }

    uint8_t AnalogStick::getY() {
        return yValue;
    }

    void AnalogStick::setButtons(ButtonAnalog* up, ButtonAnalog* down, ButtonAnalog* left, ButtonAnalog* right,
                                 Button* button) {
        if (this->up) delete this->up;
        if (this->down) delete this->down;
        if (this->left) delete this->left;
        if (this->right) delete this->right;

        this->up    = up ? up : new ButtonAnalog();
        this->down  = down ? down : new ButtonAnalog();
        this->left  = left ? left : new ButtonAnalog();
        this->right = right ? right : new ButtonAnalog();

        this->button = button ? button : new Button();

        setLogic(this->logic, this->tolerance);
    }

    void AnalogStick::setLogic(uint16_t logic) {
        setLogic(logic, tolerance);
    }

    void AnalogStick::setLogic(uint16_t logic, uint8_t tolerance) {
        this->logic     = logic;
        this->tolerance = tolerance;

        uint16_t difference = (double)logic * ((double)tolerance / double(100));

        up->setBounds(0, difference);
        down->setBounds(logic - difference, logic);
        left->setBounds(0, difference);
        right->setBounds(logic - difference, logic);
    }

    void AnalogStick::setUpdateInterval(uint32_t updateInterval) {
        button->setUpdateInterval(updateInterval);
        up->setUpdateInterval(updateInterval);
        down->setUpdateInterval(updateInterval);
        left->setUpdateInterval(updateInterval);
        right->setUpdateInterval(updateInterval);
    }

    void AnalogStick::setDefaultMinPushTime(uint32_t defaultMinPushTime) {
        button->setDefaultMinPushTime(defaultMinPushTime);
        up->setDefaultMinPushTime(defaultMinPushTime);
        down->setDefaultMinPushTime(defaultMinPushTime);
        left->setDefaultMinPushTime(defaultMinPushTime);
        right->setDefaultMinPushTime(defaultMinPushTime);
    }

    void AnalogStick::setDefaultMinReleaseTime(uint32_t defaultMinReleaseTime) {
        button->setDefaultMinReleaseTime(defaultMinReleaseTime);
        up->setDefaultMinReleaseTime(defaultMinReleaseTime);
        down->setDefaultMinReleaseTime(defaultMinReleaseTime);
        left->setDefaultMinReleaseTime(defaultMinReleaseTime);
        right->setDefaultMinReleaseTime(defaultMinReleaseTime);
    }

    void AnalogStick::setDefaultTimeSpan(uint32_t defaultTimeSpan) {
        button->setDefaultTimeSpan(defaultTimeSpan);
        up->setDefaultTimeSpan(defaultTimeSpan);
        down->setDefaultTimeSpan(defaultTimeSpan);
        left->setDefaultTimeSpan(defaultTimeSpan);
        right->setDefaultTimeSpan(defaultTimeSpan);
    }

    void AnalogStick::setDefaultHoldTime(uint32_t defaultHoldInterval) {
        button->setDefaultHoldTime(defaultHoldInterval);
        up->setDefaultHoldTime(defaultHoldInterval);
        down->setDefaultHoldTime(defaultHoldInterval);
        left->setDefaultHoldTime(defaultHoldInterval);
        right->setDefaultHoldTime(defaultHoldInterval);
    }
}