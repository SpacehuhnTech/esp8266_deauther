#ifndef SimpleButton_ClickEvent_h
#define SimpleButton_ClickEvent_h

#include "Event.h"

namespace simplebutton {
    class ClickEvent : public Event {
        public:
            ClickEvent(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime);
            ~ClickEvent();

            uint8_t getMode();
            uint32_t getMinPushTime();
            uint32_t getMinReleaseTime();

        private:
            uint32_t minPushTime    = 0;
            uint32_t minReleaseTime = 0;
    };
}

#endif // ifndef SimpleButton_ClickEvent_h