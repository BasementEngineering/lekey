#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "led_output.h"
#include "pins.h"

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

#include "encoder_input.h"
#include "button_input.h"

Encoder_Input encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN);
Button_Input buttonInput(ENCODER_SW_PIN, 20); // Debounce delay 50ms
UserInterface ui(&display, &playback_speed_bpm, &encoder, &buttonInput);
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

void main_button_isr(){
  buttonInput.handleInterrupt();
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

    encoder.begin();
    buttonInput.begin(main_button_isr);

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
// Remove encoder.process() from loop and use a timer interrupt instead
}