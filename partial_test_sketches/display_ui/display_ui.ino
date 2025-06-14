#include "display_output.h"

Display_Output display;

void setup() {
    Serial.begin(115200);
    Serial.println("Display UI Example");

    display.begin();
    display.update(); // Initial display update
}

void loop() {
    display.update(); // Update the display in the loop
}