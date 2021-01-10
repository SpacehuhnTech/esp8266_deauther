#ifndef SimpleButton_ButtonAnalog_h
#define SimpleButton_ButtonAnalog_h

#include "Button.h"


namespace simplebutton {
    class ButtonAnalog : public Button {
        public:
            ButtonAnalog();
            ButtonAnalog(uint8_t pin);
            ButtonAnalog(uint16_t minValue, uint16_t maxValue);
            ButtonAnalog(uint8_t pin, uint16_t minValue, uint16_t maxValue);

            ~ButtonAnalog();

            void setup(uint8_t pin, uint16_t minValue, uint16_t maxValue);

            int read();

            void update();
            void update(int state);

            void setMin(uint16_t minValue);
            void setMax(uint16_t maxValue);
            void setBounds(uint16_t minValue, uint16_t maxValue);

            uint16_t getValue();
            void setValue(int value);

        private:
            uint16_t minValue = 0;
            uint16_t maxValue = 1024;
            uint16_t value    = 0;
    };
}
#endif // ifndef SimpleButton_ButtonAnalog_h
