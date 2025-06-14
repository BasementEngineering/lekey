#define CLK_PIN 18
#define DT_PIN 19
#define SW_PIN 20

#include "encoder_input.h"
#include "button_input.h"

Encoder_Input encoder(CLK_PIN, DT_PIN);
Button_Input buttonInput(SW_PIN, 50, 400); // Debounce delay 50ms, double click delay 400ms

void setup() {
    // Initialize pins
    encoder.begin();
    buttonInput.begin();
    Serial.begin(115200);
}

void loop() {
    // Rotary encoder rotation
    int rotation = encoder.read();
    if (rotation != 0) {
        if (rotation == 1) {
            Serial.println("Rotated right");
        } else if (rotation == -1) {
            Serial.println("Rotated left");
        }
    }

    // Button handling
    int buttonState = buttonInput.update();
    if (buttonState == Button_Input::SINGLE_CLICK) {
        Serial.println("Button single clicked");
    } else if (buttonState == Button_Input::DOUBLE_CLICK) {
        Serial.println("Button double clicked");
    }
}