#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "pins.h"

int playback_speed_bpm = 120; // Playback speed in beats per minute
/*
char* tune_list[] = {
    "6LEDS.MID", // Simple and short file
    NULL // End of list
};

*/
int choosen_tune = 0; // Index of the currently chosen tune

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
    Serial.print("Lit note: ");
    Serial.println(midi_note);
  } else if (pev->data[0] == 0x80) { // Note Off event
    uint8_t midi_note = pev->data[1]; // MIDI note number
    Serial.print("Cleared note: ");
    Serial.println(midi_note);
  }
  Serial.println();
}

unsigned long tick_length = 100;
unsigned long last_tick_time = 0;

unsigned long current_tick = 0;
unsigned long tick_duration_ms = 0;

unsigned long last_tick_count = 0; 

int last_bpm = 0; // Last BPM value to avoid unnecessary updates

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Player");
    delay(1000);
    
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

    tick_duration_us = SMF.getTickTime(); // Get the tick duration in milliseconds
    unsigned long ticks_per_quarter_note = SMF.getTicksPerQuarterNote(); // Get the ticks per quarter note
    unsigned long tempo = SMF.getTempo(); // Get the current tempo in microseconds per quarter note

    Serial.print("Tick duration: ");
    Serial.print(tick_duration_us);
    Serial.println(" us");

    Serial.print("Ticks per quarter note: ");
    Serial.println(ticks_per_quarter_note);
    Serial.print("Tempo: ");
    Serial.print(tempo);
    Serial.println(" microseconds per quarter note");

    Serial.println("Ready");
}

void loop() {
    if (millis() - last_tick_time >= tick_length) {
        last_tick_time = millis();
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
    int tick_delta = current_tick - last_tick_count; // Calculate the delta since the last tick
    SMF.getNextEvent();
    last_tick_count = current_tick; // Update last tick count
  }
}