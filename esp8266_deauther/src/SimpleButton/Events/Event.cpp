#include "Event.h"

namespace simplebutton {
    Event::~Event() {
        if (next) {
            delete next;
            next = NULL;
        }
    }

    void Event::run() {
        if (fnct) fnct();
    }

    uint8_t Event::getMode() {
        return MODE::NONE;
    }

    uint32_t Event::getMinPushTime() {
        return 0;
    }

    uint32_t Event::getMinReleaseTime() {
        return 0;
    }

    uint32_t Event::getTimeSpan() {
        return 0;
    }

    uint32_t Event::getInterval() {
        return 0;
    }
}