#ifndef SimpleButton_RotaryEncoder_h
#define SimpleButton_RotaryEncoder_h

#include "Button.h"
#include "ButtonPullup.h"
#include "ButtonGPIOExpander.h"
#include "ButtonPullupGPIOExpander.h"

namespace simplebutton {
    class RotaryEncoder {
        public:
            Button* button        = NULL;
            Button* clockwise     = NULL;
            Button* anticlockwise = NULL;

            RotaryEncoder();
            RotaryEncoder(uint8_t channelA, uint8_t channelB, uint8_t button);
            RotaryEncoder(GPIOExpander* pcf, uint8_t channelA, uint8_t channelB, uint8_t button);
            RotaryEncoder(Button* clockwise, Button* anticlockwise, Button* button);

            ~RotaryEncoder();

            void setup(uint8_t channelA, uint8_t channelB, uint8_t button);
            void setup(GPIOExpander* pcf, uint8_t channelA, uint8_t channelB, uint8_t button);
            void setup(Button* clockwise, Button* anticlockwise, Button* button);

            void update();
            void update(bool stateA, bool stateB, bool buttonState);

            void reset();

            int32_t getPos();

            void setButtons(Button* clockwise, Button* anticlockwise, Button* button);
            void setPos(int32_t pos);
            void enableLoop(bool loop);
            void setEncoding(uint8_t steps);
            void setMin(int32_t value);
            void setMax(int32_t value);
            void setInverted(bool inverted);

            bool clicked();
            bool incremented();
            bool decremented();
            bool minVal();
            bool maxVal();

        private:
            int32_t pos = 0;

            bool prevA = false;
            bool prevB = false;

            enum State { STILL = 0, CLOCKWISE = 1, ANTICLOCKWISE = 2 };
            State curState  = State::STILL;
            State prevState = State::STILL;

            uint8_t button_steps = 1; // how many steps per turn (encoding)
            uint8_t steps        = 0; // tmp counter

            int32_t min   = -128;
            int32_t max   = 127;
            bool loop     = false;
            bool inverted = false;

            void goClockwise();
            void goAnticlockwise();
    };
}

#endif // ifndef SimpleButton_RotaryEncoder_h