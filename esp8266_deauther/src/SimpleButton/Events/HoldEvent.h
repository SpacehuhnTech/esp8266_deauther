#ifndef SimpleButton_HoldEvent_h
#define SimpleButton_HoldEvent_h

#include "Event.h"
namespace simplebutton {
    class HoldEvent : public Event {
        public:
            HoldEvent(ButtonEventFunction, uint32_t interval);
            ~HoldEvent();

            uint8_t getMode();
            uint32_t getInterval();

        private:
            uint32_t interval = 0;
    };
}

#endif // ifndef SimpleButton_HoldEvent_h