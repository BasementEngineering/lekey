#include "encoder_input.h"
#include "button_input.h"
#include "pins.h"

Encoder_Input encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN);
Button_Input buttonInput(ENCODER_SW_PIN, 20); // Debounce delay 50ms

void main_button_isr(){
  buttonInput.handleInterrupt();
}

void setup() {
    // Initialize pins
    encoder.begin();

    buttonInput.begin(main_button_isr);
    Serial.begin(9600);
}

void loop() {
    delay(1000);
    // Rotary encoder rotation
    int rotation = encoder.read();
    if (rotation != 0) {
        if (rotation > 0) {
            Serial.println("Rotated right");
            Serial.print("Rotation steps: ");
            Serial.println(rotation);
        } else if (rotation < 0) {
            Serial.println("Rotated left");
            Serial.print("Rotation steps: ");
            Serial.println(rotation);
        }
    }

    // Button handling
    int buttonState = buttonInput.update();
    if (buttonState == Button_Input::SINGLE_CLICK) {
        Serial.println("Button single clicked");
    } 
    else if (buttonState == Button_Input::HELD_DOWN) {
        Serial.println("Button held down");
    }
}