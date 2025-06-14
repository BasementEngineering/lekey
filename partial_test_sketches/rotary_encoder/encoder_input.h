#ifndef ENCODER_INPUT_H
#define ENCODER_INPUT_H

#include <Arduino.h>

class Encoder_Input {
public:
    Encoder_Input(uint8_t clkPin, uint8_t dtPin)
        : clkPin(clkPin), dtPin(dtPin), lastClkState(HIGH) {}
        static const unsigned long debounceDelay = 5; // milliseconds
        unsigned long lastDebounceTime = 0;

    void begin() {
        pinMode(clkPin, INPUT_PULLUP);
        pinMode(dtPin, INPUT_PULLUP);
        lastClkState = digitalRead(clkPin);
    }

    // Returns 1 for right turn, -1 for left turn, 0 for no turn
    int read() {
        if (millis() - lastDebounceTime < debounceDelay) {
            return 0; // Ignore if within debounce delay
        }
        int clkState = digitalRead(clkPin);
        int dtState = digitalRead(dtPin);
        int result = 0;

        if (clkState != lastClkState) {
            if (clkState == LOW) {
                if (dtState != clkState) {
                    result = 1; // Right turn
                } else {
                    result = -1; // Left turn
                }
            }
            lastClkState = clkState;
        }
        lastDebounceTime = millis(); // Update debounce time
        return result;
    }

private:
    uint8_t clkPin;
    uint8_t dtPin;
    int lastClkState;
};

#endif // ENCODER_INPUT_H
