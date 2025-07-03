#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "led_output.h"
#include "pins.h"

/***** PIN DEFINITIONS ********/
 /* MicroSD SPI Pins
 * MicroSD MISO pin to Pico GPIO-12
 * MicroSD MOSI pin to Pico GPIO-11
 * MicroSD SCK pin to Pico GPIO-10
 * MicroSD CS pin to Pico GPIO-13
 */

int playback_speed_bpm = 120; // Playback speed in beats per minute
/*
char* tune_list[] = {
    "6LEDS.MID", // Simple and short file
    NULL // End of list
};

*/

int choosen_tune = 0; // Index of the currently chosen tune

// OLED Dsiplay
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include "user_interface.h"

UserInterface ui(&display, &playback_speed_bpm);

#include <CtrlEnc.h>
// Forward declarations for encoder callbacks
void encoderTurnLeft();
void encoderTurnRight();

CtrlEnc encoder(14, 15, encoderTurnLeft, encoderTurnRight); // Encoder on pins 2 and 16

// Callback implementations that call the ui methods
void encoderTurnLeft() {
    ui.onTurnLeft();
}
void encoderTurnRight() {
    ui.onTurnRight();
}

LedStrip strip(LED_PIN); // Create a LedStrip object with 61 LEDs (5 octaves + 1 for the highest note)

SdFat	SD;
MD_MIDIFile SMF;

// The files in the tune list should be located on the SD card 
// or an error will occur opening the file and the next in the 
// list will be opened (skips errors).
const char *loopfile = "LeiseRieseltDerSchnee.mid";  // simple and short file

void midiCallback(midi_event *pev) {
    Serial.print("MIDI Event: ");
    Serial.print("Track: ");
    Serial.println(pev->track);
    Serial.print("Channel: ");
    Serial.println(pev->channel);
      Serial.println(" Data");
  for (uint8_t i=0; i<pev->size; i++)
  {
     Serial.print(pev->data[i],HEX);
     Serial.print(" ");
  }

  if(pev->data[0] == 0x90) { // Note On event
    uint8_t midi_note = pev->data[1]; // MIDI note number
    strip.foreshadow(midi_note); // Foreshadow the note
    strip.light(midi_note); // Light the note
    Serial.print("Lit note: ");
    Serial.println(midi_note);
  } else if (pev->data[0] == 0x80) { // Note Off event
    uint8_t midi_note = pev->data[1]; // MIDI note number
    strip.clear(); // Clear the strip for this note
    Serial.print("Cleared note: ");
    Serial.println(midi_note);
  }
  Serial.println();
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Keyboard");
    delay(1000);
    strip.begin(); // Initialize the LED strip
    strip.runTestSequence();
    strip.setMainColor(strip.Color(0, 0, 255)); // Set main color to white
    strip.setForeshadowColor(strip.Color(127, 0, 0)); // Set foreshadow color to dim white

    ui.begin(); // Initialize the user interface
    
    #define SPI_SPEED SD_SCK_MHZ(4) //Does not work without that
    if (!SD.begin(SD_CS_PIN, SPI_SPEED))
    {
        Serial.println("\nSD init fail!");
        while (true) ;
    }

    // Initialize MIDIFile
    SMF.begin(&SD);
    SMF.setMidiHandler(midiCallback);
    SMF.looping(true);

    int err;
    err = SMF.load(loopfile);
  if (err != MD_MIDIFile::E_OK)
  {
    Serial.println("SMF load Error ");
    
    while (true);
  }

    Serial.println("Ready");
}

unsigned long tick_length = 100;
unsigned long last_tick = 0;

unsigned long current_tick = 0;

int last_bpm = 0; // Last BPM value to avoid unnecessary updates

void loop() {

    if (millis() - last_tick >= tick_length) {
        last_tick = millis();
        current_tick++;
    }

      // play the file
    if(last_bpm != playback_speed_bpm) {
        last_bpm = playback_speed_bpm; // Update last BPM
        tick_length = 60000 / playback_speed_bpm; // Calculate new tick length based on BPM
        Serial.print("Tick length updated to: ");
        Serial.println(tick_length);
        SMF.setTempo(playback_speed_bpm); // Update the tempo in the MIDI file
    }
  if (!SMF.isEOF())
  {
    SMF.getNextEvent();
  }
 ui.update(); // Update the user interface
  encoder.process(); // Update the encoder state
  //ui.update(); // Update the user interface
}

/*void get_chord_from_serial() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        String notes[LED_COUNT];
        int noteCount = 0;

        // Split input by commas
        int startIndex = 0;
        for (int i = 0; i < input.length(); i++) {
            if (input[i] == ',') {
                notes[noteCount++] = input.substring(startIndex, i);
                startIndex = i + 1;
            }
        }
        // Add last note
        if (startIndex < input.length()) {
            notes[noteCount++] = input.substring(startIndex);
        }

        lightNotes(notes, noteCount);
    }
}*/

/*// Map notes to LED indices (C=0, D=1, ..., B=11)
const char* noteNames[12] = {
    "C","C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

int hex_string_to_int(const String& hexString) {
    int value = 0;
    for (int i = 0; i < hexString.length(); i++) {
        char c = hexString.charAt(i);
        if (c >= '0' && c <= '9') {
            value = value * 16 + (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value = value * 16 + (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            value = value * 16 + (c - 'a' + 10);
        }
    }
    Serial.print("Hex string: ");
    Serial.print(hexString);
    Serial.print(" converted to int: ");
    Serial.println(value);
    return value;
}

void get_midi_command_from_serial()
{
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        Serial.print("Received MIDI command: ");
        Serial.println(input);
        //The midi command is made up of three parts: command, note, velocity
        //Those are received in 3 bytes, so we can split the input by spaces
        String parts[3];
        int partCount = 0;
        int startIndex = 0;
        for (int i = 0; i < input.length(); i++) {
            if (input[i] == ' ') {
                parts[partCount++] = input.substring(startIndex, i);
                startIndex = i + 1;
            }
        }
        int midi_note = -1; // Default to an invalid note
        // Add last part
        if (startIndex < input.length()) {
            parts[partCount++] = input.substring(startIndex);
        }
        // Parse the parts
        if (partCount >= 2) {
            String command = parts[0];
            Serial.print("midi note before: ");
            Serial.println(parts[1]);

            midi_note = hex_string_to_int(parts[1]); // Convert the note from hex string to int
            Serial.print("Command: ");
            Serial.println(command);
            Serial.print("MIDI Note: ");
            Serial.println(midi_note);
        } else {
            Serial.println("Invalid MIDI command format. Please use 'command note velocity'.");
        }

        if (midi_note >= 0 && midi_note < 128) { // Valid MIDI note range
            strip.foreshadow(midi_note); // Foreshadow the note
            strip.light(midi_note); // Light the note
            Serial.print("Lit note: ");
            Serial.println(noteNames[midi_note % 12]);
        } else {
            Serial.println("Invalid MIDI note. Please enter a value between 0 and 127.");
        }
    }
}
*/