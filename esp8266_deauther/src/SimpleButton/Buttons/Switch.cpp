#include "Switch.h"

namespace simplebutton {
    Switch::Switch() {
        button = new Button();
    }

    Switch::Switch(uint8_t pin) {
        setup(pin);
    }

    Switch::Switch(GPIOExpander* pcf, uint8_t pin) {
        setup(pcf, pin);
    }

    Switch::Switch(Button* button) {
        setup(button);
    }

    Switch::~Switch() {
        if (this->button) delete this->button;
    }

    void Switch::setup(uint8_t pin) {
        button   = new Button(pin);
        tmpState = button->read();
    }

    void Switch::setup(GPIOExpander* pcf, uint8_t pin) {
        button   = new ButtonGPIOExpander(pcf, pin);
        tmpState = button->read();
    }

    void Switch::setup(Button* button) {
        setButton(button);
        tmpState = button->read();
    }

    void Switch::update() {
        update(button->read());
    }

    void Switch::update(bool state) {
        bool prevState = tmpState;

        tmpState = state > 0;

        if (prevState != tmpState) button->click();
    }

    void Switch::setButton(Button* button) {
        if (this->button) delete this->button;
        this->button = button ? button : new Button();
    }

    bool Switch::getState() {
        return tmpState;
    }

    bool Switch::clicked() {
        return button->clicked();
    }
}