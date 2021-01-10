#include "../SimpleButton.h"

namespace simplebutton {
    Button::Button() {
        setup(255, false);
    }

    Button::Button(uint8_t pin) {
        setup(pin, false);
    }

    Button::Button(uint8_t pin, bool inverted) {
        setup(pin, inverted);
    }

    Button::~Button() {}

    void Button::setup(uint8_t pin, bool inverted) {
        this->button_pin      = pin;
        this->button_inverted = inverted;
        enable();
    }

    void Button::enable() {
        button_enabled = true;

        if ((button_pin < 255) && !button_setup) {
            pinMode(button_pin, INPUT);
            button_setup = true;
        }
    }

    void Button::disable() {
        button_enabled = false;
    }

    void Button::reset() {
        pushedFlag   = false;
        releasedFlag = false;
        holdFlag     = false;
    }

    void Button::push() {
        if (!state) {
            state = true;

            prevPushTime    = pushTime;
            prevReleaseTime = releaseTime;
            pushedFlag      = true;

            pushTime = millis();
            holdTime = millis();
            holdFlag = false;
        }
    }

    void Button::release() {
        if (state) {
            state        = false;
            releasedFlag = true;
            releaseTime  = millis();
        }
    }

    void Button::click() {
        click(defaultMinPushTime);
    }

    void Button::click(uint32_t time) {
        push();
        pushTime = millis() - time - defaultMinReleaseTime;
        release();
        releaseTime = millis() - defaultMinReleaseTime;

        updateEvents();
    }

    int Button::read() {
        bool currentState = false;

        if (button_enabled && button_setup) {
            currentState = digitalRead(button_pin);

            if (button_inverted) currentState = !currentState;
        }

        return (int)currentState;
    }

    void Button::update() {
        if (millis() - updateTime >= updateInterval) {
            updateEvents();
            if (button_enabled && button_setup) update(read());
        }
    }

    void Button::update(int state) {
        // update time
        updateTime = millis();

        // check events
        updateEvents();

        // update state
        if (state > 0) push();
        else release();
    }

    void Button::updateEvents() {
        Event* e = this->events;

        while (e != NULL) {
            switch (e->getMode()) {
                case e->MODE::PUSHED:
                    if (this->pushed()) e->run();
                    break;

                case e->MODE::RELEASED:
                    if (this->released()) e->run();
                    break;

                case e->MODE::CLICKED:
                    if (this->clicked(e->getMinPushTime(), e->getMinReleaseTime())) e->run();
                    break;

                case e->MODE::DOUBLECLICKED:
                    if (this->doubleClicked(e->getMinPushTime(), e->getMinReleaseTime(), e->getTimeSpan())) e->run();
                    break;

                case e->MODE::HOLDING:
                    if (this->holding(e->getInterval())) e->run();
                    break;
            }
            e = e->next;
        }
    }

    bool Button::isInverted() {
        return button_inverted;
    }

    bool Button::isEnabled() {
        return button_enabled;
    }

    bool Button::isSetup() {
        return button_setup;
    }

    bool Button::getState() {
        return state;
    }

    int Button::getClicks() {
        return (int)clicks;
    }

    int Button::getPushTime() {
        return (int)(millis() - pushTime);
    }

    bool Button::pushed() {
        if (pushedFlag) {
            pushedFlag = false;
            return true;
        }
        return false;
    }

    bool Button::released() {
        if (releasedFlag && (pushTime < releaseTime)) {
            releasedFlag = false;
            return true;
        }
        return false;
    }

    bool Button::clicked() {
        return clicked(defaultMinPushTime);
    }

    bool Button::clicked(uint32_t minPushTime) {
        return clicked(minPushTime, defaultMinReleaseTime);
    }

    bool Button::clicked(uint32_t minPushTime, uint32_t minReleaseTime) {
        bool notHolding     = !holdFlag;
        bool minTime        = millis() - pushTime >= minPushTime;
        bool releaseTimeout = millis() - releaseTime >= minReleaseTime;

        if (notHolding && minTime && releaseTimeout) {
            if (released()) {
                clicks++;
                return true;
            }
        }

        return false;
    }

    bool Button::doubleClicked() {
        return doubleClicked(defaultMinPushTime);
    }

    bool Button::doubleClicked(uint32_t minPushTime) {
        return doubleClicked(minPushTime, defaultTimeSpan);
    }

    bool Button::doubleClicked(uint32_t minPushTime, uint32_t timeSpan) {
        return doubleClicked(minPushTime, defaultMinReleaseTime, timeSpan);
    }

    bool Button::doubleClicked(uint32_t minPushTime, uint32_t minReleaseTime, uint32_t timeSpan) {
        bool wasPrevClicked = prevReleaseTime - prevPushTime >= minPushTime;
        bool inTimeSpan     = millis() - prevPushTime <= timeSpan;
        bool releaseTimeout = millis() - prevReleaseTime >= minReleaseTime;

        if (wasPrevClicked && inTimeSpan && releaseTimeout) {
            if (clicked(minPushTime)) {
                pushTime = 0;
                return true;
            }
        }

        return false;
    }

    bool Button::holding() {
        return holding(defaultHoldInterval);
    }

    bool Button::holding(uint32_t interval) {
        if (getState() && (millis() - holdTime >= interval)) {
            holdTime = millis();
            holdFlag = true;
            return true;
        }
        return false;
    }

    void Button::setUpdateInterval(uint32_t updateInterval) {
        this->updateInterval = updateInterval;
    }

    void Button::setDefaultMinPushTime(uint32_t defaultMinPushTime) {
        this->defaultMinPushTime = defaultMinPushTime;
    }

    void Button::setDefaultMinReleaseTime(uint32_t defaultMinReleaseTime) {
        this->defaultMinReleaseTime = defaultMinReleaseTime;
    }

    void Button::setDefaultTimeSpan(uint32_t defaultTimeSpan) {
        this->defaultTimeSpan = defaultTimeSpan;
    }

    void Button::setDefaultHoldTime(uint32_t defaultHoldInterval) {
        this->defaultHoldInterval = defaultHoldInterval;
    }

    void Button::setOnPushed(ButtonEventFunction) {
        this->addEvent(new PushEvent(fnct));
    }

    void Button::setOnReleased(ButtonEventFunction) {
        this->addEvent(new ReleaseEvent(fnct));
    }

    void Button::setOnClicked(ButtonEventFunction) {
        setOnClicked(fnct, defaultMinPushTime, defaultMinReleaseTime);
    }

    void Button::setOnClicked(ButtonEventFunction, uint32_t minPushTime) {
        setOnClicked(fnct, minPushTime, defaultMinReleaseTime);
    }

    void Button::setOnClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime) {
        this->addEvent(new ClickEvent(fnct, minPushTime, minReleaseTime));
    }

    void Button::setOnDoubleClicked(ButtonEventFunction) {
        setOnDoubleClicked(fnct, defaultMinPushTime, defaultMinReleaseTime, defaultTimeSpan);
    }

    void Button::setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime) {
        setOnDoubleClicked(fnct, minPushTime, defaultMinReleaseTime, defaultTimeSpan);
    }

    void Button::setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t timeSpan) {
        setOnDoubleClicked(fnct, minPushTime, defaultMinReleaseTime, timeSpan);
    }

    void Button::setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime, uint32_t timeSpan) {
        this->addEvent(new DoubleclickEvent(fnct, minPushTime, minReleaseTime, timeSpan));
    }

    void Button::setOnHolding(ButtonEventFunction) {
        setOnHolding(fnct, defaultHoldInterval);
    }

    void Button::setOnHolding(ButtonEventFunction, uint32_t interval) {
        this->addEvent(new HoldEvent(fnct, interval));
    }

    void Button::clearEvents() {
        delete events;
        events = NULL;
    }

    void Button::addEvent(Event* e) {
        if (events == NULL) events = e;
        else {
            Event* tmp = events;

            while (tmp->next != NULL) tmp = tmp->next;
            tmp->next = e;
        }
    }
}