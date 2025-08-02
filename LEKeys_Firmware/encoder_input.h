#ifndef ENCODER_INPUT_H
#define ENCODER_INPUT_H

#include <Arduino.h>

class Encoder_Input {
public:
    Encoder_Input(uint8_t clkPin, uint8_t dtPin)
        : clkPin(clkPin), dtPin(dtPin), position(0), lastClkState(HIGH) {}

    void begin() {
        pinMode(clkPin, INPUT_PULLUP);
        pinMode(dtPin, INPUT_PULLUP);
        lastClkState = digitalRead(clkPin);
        instance = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), isr, CHANGE);
    }

    // Returns the number of steps since last call, resets internal counter
    int read() {
        noInterrupts();
        int delta = position;
        position = 0;
        interrupts();
        return delta;
    }

private:
    volatile int position;
    uint8_t clkPin;
    uint8_t dtPin;
    volatile int lastClkState;

    static Encoder_Input* instance;

    static void isr() {
        if (instance) instance->handleInterrupt();
    }

    void handleInterrupt() {
        int clkState = digitalRead(clkPin);
        int dtState = digitalRead(dtPin);

        bool clkRising = (clkState == HIGH && lastClkState == LOW);
        bool clkFalling = (clkState == LOW && lastClkState == HIGH);

        if (clkRising) {
            if (dtState == LOW) {
                position--;
            } else {
                position++;
            }
        } else if (clkFalling)
        {
            if(dtState == LOW) {
                position++;
            } else {
                position--;
            }
        }

        lastClkState = clkState;
        /*if (clkState != lastClkState) {
            if (clkState == LOW) {
                if (dtState != clkState) {
                    position++;
                } else {
                    position--;
                }
            }
            lastClkState = clkState;
        }*/
    }
};

// Static member initialization
Encoder_Input* Encoder_Input::instance = nullptr;

#endif // ENCODER_INPUT_H
