#include "ButtonPullup.h"

namespace simplebutton {
    ButtonPullup::ButtonPullup() {
        setup(255);
    }

    ButtonPullup::ButtonPullup(uint8_t pin) {
        setup(pin);
    }

    ButtonPullup::~ButtonPullup() {}

    void ButtonPullup::setup(uint8_t pin) {
        this->button_pin      = pin;
        this->button_inverted = true;
        enable();
    }

    void ButtonPullup::enable() {
        button_enabled = true;

        if ((button_pin < 255) && !button_setup) {
            pinMode(button_pin, INPUT_PULLUP);
            button_setup = true;
        }
    }
}