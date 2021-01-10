#ifndef SimpleButton_AnalogStick_h
#define SimpleButton_AnalogStick_h

#include "ButtonPullup.h"
#include "ButtonAnalog.h"

namespace simplebutton {
    class AnalogStick {
        public:
            Button* button      = NULL;
            ButtonAnalog* up    = NULL;
            ButtonAnalog* down  = NULL;
            ButtonAnalog* left  = NULL;
            ButtonAnalog* right = NULL;

            AnalogStick();
            AnalogStick(uint8_t xPin, uint8_t yPin, uint8_t buttonPin);

            ~AnalogStick();

            void setup(uint8_t xPin, uint8_t yPin, uint8_t buttonPin);

            void update();
            void update(uint8_t xValue, uint8_t yValue, bool buttonPress);

            uint8_t getX();
            uint8_t getY();

            void setButtons(ButtonAnalog* up, ButtonAnalog* down, ButtonAnalog* left, ButtonAnalog* right,
                            Button* button);
            void setLogic(uint16_t logic);
            void setLogic(uint16_t logic, uint8_t tolerance);

            void setUpdateInterval(uint32_t updateInterval);
            void setDefaultMinPushTime(uint32_t defaultMinPushTime);
            void setDefaultMinReleaseTime(uint32_t defaultMinReleaseTime);
            void setDefaultTimeSpan(uint32_t defaultTimeSpan);
            void setDefaultHoldTime(uint32_t defaultHoldInterval);

        private:
            uint16_t logic    = 1024;
            uint8_t tolerance = 25; // percentage

            uint8_t xValue = 0;
            uint8_t yValue = 0;

            uint8_t xPin      = 0;
            uint8_t yPin      = 0;
            uint8_t buttonPin = 0;
    };
}

#endif // ifndef SimpleButton_AnalogStick_h