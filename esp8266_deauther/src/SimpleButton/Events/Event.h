#ifndef SimpleButton_Event_h
#define SimpleButton_Event_h

#include "Arduino.h"
#include <functional>

#define ButtonEventFunction std::function<void()>fnct

namespace simplebutton {
    class Event {
        public:
            Event* next = NULL;
            enum MODE { NONE = 0, PUSHED = 1, RELEASED = 2, CLICKED = 3, DOUBLECLICKED = 4, HOLDING = 5 };

            virtual ~Event();

            virtual void run();

            virtual uint8_t getMode();
            virtual uint32_t getMinPushTime();
            virtual uint32_t getMinReleaseTime();
            virtual uint32_t getTimeSpan();
            virtual uint32_t getInterval();

        protected:
            ButtonEventFunction = NULL;
    };
}

#endif // ifndef Event_h