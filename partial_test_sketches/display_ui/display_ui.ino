#include "display_output.h"

Display_Output display;

void setup() {
    Serial.begin(115200);
    Serial.println("Display UI Example");

    display.begin();
    display.update(); // Initial display update
}



int bpm = 120; // Beats per minute
int beatsPerBar = 4; // Beats per measure
int currentBeat = 0; // Current beat in the measure
int currentBar = 0; // Current bar in the song
int maxBars = 120; // Total number of bars in the song
int loopLength = 3; // Length of the loop in bars
int loopStart = 4;

void updataValues(){
    currentBeat++;
    if (currentBeat % beatsPerBar == 0) {
        currentBar++;
        if (currentBar >= maxBars) {
            currentBar = 0; // Reset to the first bar
            currentBeat = 0; // Reset to the first beat
        }
    }
}

enum class DisplayState {
    INITIALIZING,
    PAUSED,
    PLAYING
};

unsigned long lastUpdateTime = 0;
DisplayState currentState = DisplayState::INITIALIZING;
char* currentFilename = "Wonderwall extra long dong.mid"; // Current filename being played

void stateMachine() {
    switch (currentState)  // Switch statement to handle different states
    {
    case DisplayState::INITIALIZING:
        if (millis() - lastUpdateTime > 1000) { // Update every second
            display.showInitializing();
            lastUpdateTime = millis();
            currentState = DisplayState::PAUSED; // Move to paused state after initializing
        }
        break;
    case DisplayState::PAUSED:
        if (millis() - lastUpdateTime > 1000) { // Update every second
                display.showPlaying(bpm, currentFilename, false,currentBar, maxBars, currentBeat, loopStart, loopLength);
                lastUpdateTime = millis();
                if( millis() > 5000) { // After 5 seconds, switch to playing state
                    currentState = DisplayState::PLAYING; // Switch to playing state after 5 seconds
                }
            }
        break;
    case DisplayState::PLAYING:
        if (millis() - lastUpdateTime > 200) { // Update every second
            updataValues();
            display.showPlaying(bpm, currentFilename, true, currentBar, maxBars, currentBeat, loopStart, loopLength);
            lastUpdateTime = millis();
        }
        break;
    
    default:
        break;
    }
}

void loop() {
    //display.update(); // Update the display in the loop
    stateMachine();
}