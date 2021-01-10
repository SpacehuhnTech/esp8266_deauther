#include "ReleaseEvent.h"

namespace simplebutton {
    ReleaseEvent::ReleaseEvent(ButtonEventFunction) {
        this->fnct = fnct;
    }

    ReleaseEvent::~ReleaseEvent() {
        if (next) {
            delete next;
            next = NULL;
        }
    }

    uint8_t ReleaseEvent::getMode() {
        return MODE::RELEASED;
    }
}