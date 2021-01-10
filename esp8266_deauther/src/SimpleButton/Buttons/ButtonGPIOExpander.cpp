#include "ButtonGPIOExpander.h"

namespace simplebutton {
    ButtonGPIOExpander::ButtonGPIOExpander() {
        setup(NULL, 255, false);
    }

    ButtonGPIOExpander::ButtonGPIOExpander(GPIOExpander* pcf, uint8_t pin) {
        setup(pcf, pin, false);
    }

    ButtonGPIOExpander::ButtonGPIOExpander(GPIOExpander* pcf, uint8_t pin, bool inverted) {
        setup(pcf, pin, inverted);
    }

    ButtonGPIOExpander::~ButtonGPIOExpander() {}

    void ButtonGPIOExpander::setup(GPIOExpander* pcf, uint8_t pin, bool inverted) {
        this->pcf             = pcf;
        this->button_pin      = pin;
        this->button_inverted = inverted;
        enable();
    }

    void ButtonGPIOExpander::enable() {
        button_enabled = true;

        if (pcf) {
            pcf->write(button_pin, 0);
            if (pcf->connected()) button_setup = true;
        }
    }

    int ButtonGPIOExpander::read() {
        bool currentState = false;

        if (button_enabled && button_setup) {
            currentState = pcf->read(button_pin) > 0;

            if (button_inverted) currentState = !currentState;
        }

        return (int)currentState;
    }

    void ButtonGPIOExpander::update() {
        if (button_enabled && button_setup) {
            update(read());
        }
    }

    void ButtonGPIOExpander::update(int state) {
        Button::update(state);
    }
}
