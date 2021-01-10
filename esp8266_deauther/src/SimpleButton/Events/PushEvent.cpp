#include "PushEvent.h"

namespace simplebutton {
    PushEvent::PushEvent(ButtonEventFunction) {
        this->fnct = fnct;
    }

    PushEvent::~PushEvent() {
        if (next) {
            delete next;
            next = NULL;
        }
    }

    uint8_t PushEvent::getMode() {
        return MODE::PUSHED;
    }
}