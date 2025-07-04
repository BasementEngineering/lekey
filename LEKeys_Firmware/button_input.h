#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>

class Button_Input {
public:
    Button_Input(uint8_t pin, unsigned long debounceDelay = 50, unsigned long heldDownDelay = 500)
        : swPin(pin),
          debounceDelay(debounceDelay),
          heldDownDelay(heldDownDelay),
          lastButtonPressTime(0),
          lastPressState(false),
          waitingForDouble(false),
          eventFlag(NONE)
    {}

    enum ButtonState {
        NONE = 0,
        SINGLE_CLICK = 1,
        HELD_DOWN = -1
    };

    void begin(void (*isr)()) {
        attachInterrupt(digitalPinToInterrupt(swPin), isr, CHANGE);
        //Setting ISR resets pullup
        pinMode(swPin, INPUT_PULLUP);
    }

    // Call this in loop(), returns:
    // 0 = nothing, 1 = single click, 2 = double click, -1 = button held down (optional)
    int update() {
        noInterrupts();

        if(lastPressState) {
            unsigned long now = millis();
            if( (now - lastButtonPressTime) > heldDownDelay) {
                eventFlag = HELD_DOWN; // Button held down
            }
        }
        int event = eventFlag;
        eventFlag = NONE;

        interrupts();
        return event;
    }

    // This should be called from the ISR
    void handleInterrupt() {
        unsigned long now = millis();
        bool currentPressState = !digitalRead(swPin);
        
        //Button click start
        if (!lastPressState && currentPressState) {
          lastButtonPressTime = now;
        }
        //Button click end
        else if (lastPressState && !currentPressState) {
          if((now - lastButtonPressTime) < heldDownDelay) {
                  eventFlag = SINGLE_CLICK;
              }
          }
          lastPressState = currentPressState;
        
    }

    bool isButtonPressed() const {
        return lastPressState;
    }

private:
    uint8_t swPin;
    unsigned long debounceDelay;
    unsigned long heldDownDelay;
    volatile unsigned long lastButtonPressTime;
    volatile bool lastPressState;
    volatile bool waitingForDouble;
    volatile int eventFlag;
};

#endif // BUTTON_INPUT_H