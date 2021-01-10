#include "PS2Gamepad.h"

namespace simplebutton {
  #include "PS2Gamepad.h"

    PS2Gamepad::PS2Gamepad() {}

    PS2Gamepad::PS2Gamepad(uint8_t clockPin, uint8_t cmdPin, uint8_t attPin, uint8_t dataPin, bool analog) {
        setup(clockPin, cmdPin, attPin, dataPin, analog);
    }

    PS2Gamepad::~PS2Gamepad() {
        if (up) delete up;
        if (down) delete down;
        if (left) delete left;
        if (right) delete right;

        if (l1) delete l1;
        if (l2) delete l2;
        if (r1) delete r1;
        if (r2) delete r2;

        if (square) delete square;
        if (triangle) delete triangle;
        if (cross) delete cross;
        if (circle) delete circle;

        if (select) delete select;
        if (start) delete start;

        if (analogLeft) delete analogLeft;
        if (analogRight) delete analogRight;
    }

    void PS2Gamepad::setup(uint8_t clockPin, uint8_t cmdPin, uint8_t attPin, uint8_t dataPin, bool analog) {
        // pin setup
        this->clockPin        = clockPin;
        this->cmdPin          = cmdPin;
        this->attPin          = attPin;
        this->dataPin         = dataPin;
        this->rumbleEnabled   = analog;
        this->pressureEnabled = analog;

        pinMode(clockPin, OUTPUT);
        pinMode(attPin,   OUTPUT);
        pinMode(cmdPin,   OUTPUT);

        pinMode(dataPin,  INPUT_PULLUP);

        digitalWrite(cmdPin, HIGH);
        digitalWrite(clockPin, HIGH);

        // button setup
        if (!up) up = new ButtonAnalog(1, 255);
        if (!down) down = new ButtonAnalog(1, 255);
        if (!left) left = new ButtonAnalog(1, 255);
        if (!right) right = new ButtonAnalog(1, 255);

        if (!l1) l1 = new ButtonAnalog(1, 255);
        if (!l2) l2 = new ButtonAnalog(1, 255);
        if (!r1) r1 = new ButtonAnalog(1, 255);
        if (!r2) r2 = new ButtonAnalog(1, 255);

        if (!square) square = new ButtonAnalog(1, 255);
        if (!triangle) triangle = new ButtonAnalog(1, 255);
        if (!cross) cross = new ButtonAnalog(1, 255);
        if (!circle) circle = new ButtonAnalog(1, 255);

        if (!select) select = new Button();
        if (!start) start = new Button();

        if (!analogLeft) {
            analogLeft = new AnalogStick();
            analogLeft->setLogic(256);
        }
        if (!analogRight) {
            analogRight = new AnalogStick();
            analogRight->setLogic(256);
        }

        // connection setup
        poll();
        delay(10);
        poll();
        delay(10);
        poll();

        /*
           if ((gamepadData[1] != 0x41) && (gamepadData[1] != 0x73) && (gamepadData[1] != 0x79)) {
           errorCode = 1;
           return;
           }
         */

        int  tries   = 0;
        bool success = false;

        do {
            sendEnter();

            delayMicroseconds(5);

            digitalWrite(cmdPin, HIGH);
            digitalWrite(clockPin, HIGH);
            digitalWrite(attPin, LOW); // enable joystick

            delayMicroseconds(5);

            sendRead();

            digitalWrite(attPin, HIGH); // disable joystick

            sendMode();
            if (rumbleEnabled) sendRumble();
            if (pressureEnabled) sendBytesLarge();
            sendExit();

            poll();

            if ((gamepadData[1] == 0x79) || (gamepadData[1] == 0x73)) {
                success = true;
            } else {
                tries++;
            }
        } while (tries < 10 && !success);

        if (!success) {
            errorCode = 2;
        } else {
            is_connected = true;
        }
    }

    void PS2Gamepad::update() {
        if (is_connected && (millis() - lastPoll > updateInterval)) {
            poll();

            up->update(getDigitalValue(4));
            down->update(getDigitalValue(6));
            left->update(getDigitalValue(7));
            right->update(getDigitalValue(5));

            l1->update(getDigitalValue(10));
            l2->update(getDigitalValue(8));
            r1->update(getDigitalValue(11));
            r2->update(getDigitalValue(9));

            square->update(getDigitalValue(15));
            triangle->update(getDigitalValue(12));
            cross->update(getDigitalValue(14));
            circle->update(getDigitalValue(13));

            select->update(getDigitalValue(0));
            start->update(getDigitalValue(3));

            analogLeft->update(getAnalogValue(7), getAnalogValue(8), getDigitalValue(1));
            analogRight->update(getAnalogValue(5), getAnalogValue(6), getDigitalValue(2));

            if (pressureEnabled) {
                up->setValue(getDigitalValue(4) | getAnalogValue(11));
                down->setValue(getDigitalValue(6) | getAnalogValue(12));
                left->setValue(getDigitalValue(7) | getAnalogValue(10));
                right->setValue(getDigitalValue(5) | getAnalogValue(9));

                l1->setValue(getDigitalValue(10) | getAnalogValue(17));
                l2->setValue(getDigitalValue(8) | getAnalogValue(19));
                r1->setValue(getDigitalValue(11) | getAnalogValue(18));
                r2->setValue(getDigitalValue(9) | getAnalogValue(20));

                square->setValue(getDigitalValue(15) | getAnalogValue(16));
                triangle->setValue(getDigitalValue(12) | getAnalogValue(13));
                cross->setValue(getDigitalValue(14) | getAnalogValue(15));
                circle->setValue(getDigitalValue(13) | getAnalogValue(14));
            }
        }
    }

    String PS2Gamepad::getError() {
        String msg;

        switch (errorCode) {
        case 0:
            msg += String("OK");
            break;

        case 1:
            msg += String("Mode not matched or not found [");
            msg += String(gamepadData[1], HEX);
            msg += String("]");
            break;

        case 2:
            msg += String("Not accepting commands [");
            msg += String(gamepadData[1], HEX);
            msg += String("]");
            break;
        }

        errorCode = 0;

        return msg;
    }

    bool PS2Gamepad::connected() {
        return is_connected;
    }

    void PS2Gamepad::setUpdateInterval(uint32_t updateInterval) {
        this->updateInterval = updateInterval;
    }

    void PS2Gamepad::setMotors(uint8_t motorA, uint8_t motorB) {
        if (!rumbleEnabled) {
            rumbleEnabled   = true;
            pressureEnabled = true;
            reconfig();
        }
        poll(motorA, motorB);
    }

    void PS2Gamepad::reconfig() {
        sendEnter();
        sendMode();
        if (rumbleEnabled) sendRumble();
        if (pressureEnabled) sendBytesLarge();
        sendExit();
    }

    void PS2Gamepad::poll() {
        if (millis() - lastPoll > 1500) reconfig();
        lastPoll = millis();

        uint8_t dwordA[9]  = { 0x01, 0x42, 0x00, motorA, motorB, 0x00, 0x00, 0x00, 0x00 };
        uint8_t dwordB[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        int  tries   = 0;
        bool success = false;

        do {
            digitalWrite(cmdPin, HIGH);
            digitalWrite(clockPin, HIGH);
            digitalWrite(attPin, LOW); // low enable joystick

            delayMicroseconds(5);

            // send command to ask for data
            for (int i = 0; i < 9; i++) {
                gamepadData[i] = shift(dwordA[i]);
            }

            // if controller is in full data return mode, get the rest of data
            if (gamepadData[1] == 0x79) {
                for (int i = 0; i < 12; i++) {
                    gamepadData[i + 9] = shift(dwordB[i]);
                }
            }

            digitalWrite(attPin, HIGH); // disable joystick

            // Check to see if we received valid data or not.
            // We should be in analog mode for our data to be valid (analog == 0x7_)
            if ((gamepadData[1] & 0xf0) == 0x70) {
                success = true;
            } else {
                // If we got to here, we are not in analog mode, try to recover...
                reconfig();
                tries++;
            }
        } while (tries < 10 && !success);

        prevButtonData = buttonData;

        buttonData = ((uint16_t)gamepadData[4] << 8) | gamepadData[3];
    }

    void PS2Gamepad::poll(uint8_t motorA, uint8_t motorB) {
        this->motorA = motorA;
        this->motorB = motorB;
        poll();
    }

    bool PS2Gamepad::getDigitalValue(uint8_t button) {
        if (/*button >= 0 && */ button <= 15) return (~buttonData >> button) & 1;
        else return 0;
    }

    uint8_t PS2Gamepad::getAnalogValue(uint8_t button) {
        return gamepadData[button];
    }

    uint8_t PS2Gamepad::shift(uint8_t data) {
        uint8_t tmp = 0;

        for (int i = 0; i < 8; i++) {
            if (data & (1 << i)) digitalWrite(cmdPin, HIGH);
            else digitalWrite(cmdPin, LOW);

            digitalWrite(clockPin, LOW);
            delayMicroseconds(5);

            if (digitalRead(dataPin)) bitSet(tmp, i);

            digitalWrite(clockPin, HIGH);
            delayMicroseconds(5);
        }

        digitalWrite(cmdPin, HIGH);
        delayMicroseconds(5);

        return tmp;
    }

    void PS2Gamepad::sendEnter() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x43);
        shift(0x00);
        shift(0x01);
        shift(0x00);

        digitalWrite(attPin, HIGH);
    }

    void PS2Gamepad::sendMode() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x44);
        shift(0x00);
        shift(0x01);
        shift(0x03);
        shift(0x00);
        shift(0x00);
        shift(0x00);
        shift(0x00);

        digitalWrite(attPin, HIGH);
    }

    void PS2Gamepad::sendRumble() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x4D);
        shift(0x00);
        shift(0x00);
        shift(0x01);

        digitalWrite(attPin, HIGH);
    }

    void PS2Gamepad::sendBytesLarge() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x4F);
        shift(0x00);
        shift(0xFF);
        shift(0xFF);
        shift(0x03);
        shift(0x00);
        shift(0x00);
        shift(0x00);

        digitalWrite(attPin, HIGH);
    }

    void PS2Gamepad::sendExit() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x43);
        shift(0x00);
        shift(0x00);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);

        digitalWrite(attPin, HIGH);
    }

    void PS2Gamepad::sendRead() {
        digitalWrite(attPin, LOW);

        shift(0x01);
        shift(0x45);
        shift(0x00);
        controllerType = shift(0x5A);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);
        shift(0x5A);

        digitalWrite(attPin, HIGH);
    }
}
