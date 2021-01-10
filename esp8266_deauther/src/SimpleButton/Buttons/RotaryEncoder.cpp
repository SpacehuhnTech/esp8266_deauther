#include "RotaryEncoder.h"

namespace simplebutton {
    RotaryEncoder::RotaryEncoder() {
        setButtons(NULL, NULL, NULL);
    }

    RotaryEncoder::RotaryEncoder(uint8_t channelA, uint8_t channelB, uint8_t button) {
        setup(channelA, channelB, button);
    }

    RotaryEncoder::RotaryEncoder(GPIOExpander* pcf, uint8_t channelA, uint8_t channelB, uint8_t button) {
        setup(pcf, channelA, channelB, button);
    }

    RotaryEncoder::RotaryEncoder(Button* clockwise, Button* anticlockwise, Button* button) {
        setup(clockwise, anticlockwise, button);
    }

    RotaryEncoder::~RotaryEncoder() {
        if (this->clockwise) delete this->clockwise;
        if (this->anticlockwise) delete this->anticlockwise;
        if (this->button) delete this->button;
    }

    void RotaryEncoder::setup(uint8_t channelA, uint8_t channelB, uint8_t button) {
        this->clockwise     = new ButtonPullup(channelA);
        this->anticlockwise = new ButtonPullup(channelB);
        this->button        = new ButtonPullup(button);

        prevA = clockwise->read();
        prevB = anticlockwise->read();
    }

    void RotaryEncoder::setup(GPIOExpander* pcf, uint8_t channelA, uint8_t channelB, uint8_t button) {
        this->clockwise     = new ButtonPullupGPIOExpander(pcf, channelA);
        this->anticlockwise = new ButtonPullupGPIOExpander(pcf, channelB);
        this->button        = new ButtonPullupGPIOExpander(pcf, button);

        prevA = clockwise->read();
        prevB = anticlockwise->read();
    }

    void RotaryEncoder::setup(Button* clockwise, Button* anticlockwise, Button* button) {
        setButtons(clockwise, anticlockwise, button);

        prevA = clockwise->read();
        prevB = anticlockwise->read();
    }

    void RotaryEncoder::update() {
        update(clockwise->read(), anticlockwise->read(), button->read());
    }

    void RotaryEncoder::update(bool stateA, bool stateB, bool buttonState) {
        button->update(buttonState);

        if (curState == State::STILL) {
            if ((stateA != prevA) && (stateB == prevB)) {
                prevA    = stateA;
                curState = State::ANTICLOCKWISE;
            } else if ((stateA == prevA) && (stateB != prevB)) {
                prevB    = stateB;
                curState = State::CLOCKWISE;
            }
        } else if ((curState != State::STILL) && (stateA == stateB)) {
            prevA = stateA;
            prevB = stateB;

            if (curState == prevState) steps++;
            else steps = 1;

            if (steps >= button_steps) {
                if (curState == State::CLOCKWISE) {
                    if (!inverted) goClockwise();
                    else goAnticlockwise();
                } else if (curState == State::ANTICLOCKWISE)  {
                    if (!inverted) goAnticlockwise();
                    else goClockwise();
                }

                steps = 0;
            }

            prevState = curState;
            curState  = State::STILL;
        }
    }

    void RotaryEncoder::reset() {
        button->reset();
        clockwise->reset();
        anticlockwise->reset();

        curState  = State::STILL;
        prevState = State::STILL;

        steps = 0;
    }

    int32_t RotaryEncoder::getPos() {
        return pos;
    }

    void RotaryEncoder::setButtons(Button* clockwise, Button* anticlockwise, Button* button) {
        if (this->clockwise) delete this->clockwise;
        if (this->anticlockwise) delete this->anticlockwise;
        if (this->button) delete this->button;

        this->clockwise     = clockwise ? clockwise : new Button();
        this->anticlockwise = anticlockwise ? anticlockwise : new Button();
        this->button        = button ? button : new Button();
    }

    void RotaryEncoder::setPos(int32_t pos) {
        this->pos = pos;
    }

    void RotaryEncoder::setMin(int32_t value) {
        this->min = value;
    }

    void RotaryEncoder::setMax(int32_t value) {
        this->max = value;
    }

    void RotaryEncoder::setEncoding(uint8_t steps) {
        if ((steps == 1) || (steps == 2) || (steps == 4)) this->button_steps = steps;
    }

    void RotaryEncoder::enableLoop(bool loop) {
        this->loop = loop;
    }

    void RotaryEncoder::setInverted(bool inverted) {
        this->inverted = inverted;
    }

    void RotaryEncoder::goClockwise() {
        clockwise->click();
        anticlockwise->reset();
        if (pos < max) pos++;
        else if (loop) pos = min;
    }

    void RotaryEncoder::goAnticlockwise() {
        anticlockwise->click();
        clockwise->reset();
        if (pos > min) pos--;
        else if (loop) pos = max;
    }

    bool RotaryEncoder::clicked() {
        return button->clicked();
    }

    bool RotaryEncoder::incremented() {
        return clockwise->clicked();
    }

    bool RotaryEncoder::decremented() {
        return anticlockwise->clicked();
    }

    bool RotaryEncoder::minVal() {
        return pos == min;
    }

    bool RotaryEncoder::maxVal() {
        return pos == max;
    }
}