#include "led_output.h"

LedStrip ledstrip(22); // Use pin 6 for the LED strip

void setup() {
    Serial.begin(115200);
    ledstrip.begin();

    // Test: Set foreshadow color to blue at 50%
    ledstrip.setForeshadowColor(ledstrip.Color(0, 0, 255));
    // Test: Set main color to red
    ledstrip.setMainColor(ledstrip.Color(255, 0, 0));
}

void loop() {
    // Cycle through MIDI notes 24 (C1) to 59 (B4)
    for (uint8_t midi_note = 24; midi_note <= 59; ++midi_note) {
        ledstrip.clear();
        ledstrip.foreshadow(midi_note);
        delay(200);
        ledstrip.light(midi_note);
        delay(200);
    }
    ledstrip.clear();
    delay(1000);
}