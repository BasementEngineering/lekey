#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>

class Button_Input {
public:
    Button_Input(uint8_t pin, unsigned long debounceDelay = 50, unsigned long doubleClickDelay = 300)
        : swPin(pin),
          debounceDelay(debounceDelay),
          doubleClickDelay(doubleClickDelay),
          lastButtonPress(0),
          lastClickTime(0),
          buttonPressed(false),
          waitingForDouble(false)
    {}

    enum ButtonState {
        NONE = 0,
        SINGLE_CLICK = 1,
        DOUBLE_CLICK = 2,
        HELD_DOWN = -1 // Optional, can be used if you want to detect held down state
    };

    void begin() {
        pinMode(swPin, INPUT_PULLUP);
    }

    // Call this in loop(), returns:
    // 0 = nothing, 1 = single click, 2 = double click, -1 = button held down (optional)
    int update() {
        int swState = digitalRead(swPin);
        unsigned long now = millis();

        if (swState == LOW && !buttonPressed && (now - lastButtonPress > debounceDelay)) {
            buttonPressed = true;
            lastButtonPress = now;
        }

        if (swState == HIGH && buttonPressed) {
            buttonPressed = false;
            if ((now - lastClickTime) < doubleClickDelay && waitingForDouble) {
                waitingForDouble = false;
                lastClickTime = 0;
                return 2; // Double click
            } else {
                waitingForDouble = true;
                lastClickTime = now;
            }
        }

        if (waitingForDouble && (now - lastClickTime > doubleClickDelay)) {
            waitingForDouble = false;
            lastClickTime = 0;
            return 1; // Single click
        }

        return 0; // No event
    }

    bool isButtonPressed() const {
        return buttonPressed;
    }

private:
    uint8_t swPin;
    unsigned long debounceDelay;
    unsigned long doubleClickDelay;
    unsigned long lastButtonPress;
    unsigned long lastClickTime;
    bool buttonPressed;
    bool waitingForDouble;
};

#endif // BUTTON_INPUT_H