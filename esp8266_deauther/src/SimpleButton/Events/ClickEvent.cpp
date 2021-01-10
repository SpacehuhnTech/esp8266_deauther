#include "ClickEvent.h"

namespace simplebutton {
    ClickEvent::ClickEvent(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime) {
        this->fnct           = fnct;
        this->minPushTime    = minPushTime;
        this->minReleaseTime = minReleaseTime;
    }

    ClickEvent::~ClickEvent() {
        if (next) {
            delete next;
            next = NULL;
        }
    }

    uint8_t ClickEvent::getMode() {
        return MODE::CLICKED;
    }

    uint32_t ClickEvent::getMinPushTime() {
        return minPushTime;
    }

    uint32_t ClickEvent::getMinReleaseTime() {
        return minReleaseTime;
    }
}