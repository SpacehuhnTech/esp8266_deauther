#include "HoldEvent.h"

namespace simplebutton {
    HoldEvent::HoldEvent(ButtonEventFunction, uint32_t interval) {
        this->fnct     = fnct;
        this->interval = interval;
    }

    HoldEvent::~HoldEvent() {
        if (next) {
            delete next;
            next = NULL;
        }
    }

    uint8_t HoldEvent::getMode() {
        return MODE::HOLDING;
    }

    uint32_t HoldEvent::getInterval() {
        return interval;
    }
}