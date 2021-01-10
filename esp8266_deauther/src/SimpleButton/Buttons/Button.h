#ifndef SimpleButton_Button_h
#define SimpleButton_Button_h

#include "Arduino.h"
#include "../Events/Event.h"
#include "../Events/PushEvent.h"
#include "../Events/ReleaseEvent.h"
#include "../Events/ClickEvent.h"
#include "../Events/DoubleclickEvent.h"
#include "../Events/HoldEvent.h"

namespace simplebutton {
    class Button {
        public:
            Button();
            Button(uint8_t pin);
            Button(uint8_t pin, bool inverted);

            virtual ~Button();

            void setup(uint8_t pin, bool inverted);

            virtual void enable();
            virtual void disable();
            virtual void reset();

            virtual void push();
            virtual void release();

            virtual void click();
            virtual void click(uint32_t time);

            virtual int read();
            virtual void update();
            virtual void update(int state);
            virtual void updateEvents();

            virtual bool isInverted();
            virtual bool isEnabled();
            virtual bool isSetup();

            virtual bool getState();
            virtual int getClicks();
            virtual int getPushTime();

            virtual bool pushed();
            virtual bool released();
            virtual bool clicked();
            virtual bool clicked(uint32_t minPushTime);
            virtual bool clicked(uint32_t minPushTime, uint32_t minReleaseTime);
            virtual bool doubleClicked();
            virtual bool doubleClicked(uint32_t minPushTime);
            virtual bool doubleClicked(uint32_t minPushTime, uint32_t timeSpan);
            virtual bool doubleClicked(uint32_t minPushTime, uint32_t minReleaseTime, uint32_t timeSpan);
            virtual bool holding();
            virtual bool holding(uint32_t interval);

            virtual void setUpdateInterval(uint32_t updateInterval);
            virtual void setDefaultMinPushTime(uint32_t defaultMinPushTime);
            virtual void setDefaultMinReleaseTime(uint32_t defaultMinReleaseTime);
            virtual void setDefaultTimeSpan(uint32_t defaultTimeSpan);
            virtual void setDefaultHoldTime(uint32_t defaultHoldInterval);

            virtual void setOnPushed(ButtonEventFunction);
            virtual void setOnReleased(ButtonEventFunction);
            virtual void setOnClicked(ButtonEventFunction);
            virtual void setOnClicked(ButtonEventFunction, uint32_t minPushTime);
            virtual void setOnClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime);
            virtual void setOnDoubleClicked(ButtonEventFunction);
            virtual void setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime);
            virtual void setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t timeSpan);
            virtual void setOnDoubleClicked(
                ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime, uint32_t timeSpan);
            virtual void setOnHolding(ButtonEventFunction);
            virtual void setOnHolding(ButtonEventFunction, uint32_t interval);

            virtual void clearEvents();

        protected:
            Event* events = NULL;

            bool button_inverted = false;
            bool button_setup    = false;
            bool button_enabled  = false;
            bool state           = false;
            bool pushedFlag      = false;
            bool releasedFlag    = false;
            bool holdFlag        = false;

            uint8_t button_pin = 255;

            uint16_t clicks = 0;

            uint32_t pushTime        = 0;
            uint32_t releaseTime     = 0;
            uint32_t prevPushTime    = 0;
            uint32_t prevReleaseTime = 0;
            uint32_t holdTime        = 0;
            uint32_t updateTime      = 0;

            uint32_t updateInterval        = 5;
            uint32_t defaultMinPushTime    = 40;
            uint32_t defaultMinReleaseTime = 40;
            uint32_t defaultTimeSpan       = 500;
            uint32_t defaultHoldInterval   = 500;

            void addEvent(Event* e);
    };
}

#endif // ifndef SimpleButton_Button_h