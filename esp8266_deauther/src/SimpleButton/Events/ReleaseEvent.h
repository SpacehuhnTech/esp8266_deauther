#ifndef SimpleButton_ReleaseEvent_h
#define SimpleButton_ReleaseEvent_h

#include "Event.h"

namespace simplebutton {
    class ReleaseEvent : public Event {
        public:
            ReleaseEvent(ButtonEventFunction);
            ~ReleaseEvent();

            uint8_t getMode();
    };
}

#endif // ifndef SimpleButton_ReleaseEvent_h