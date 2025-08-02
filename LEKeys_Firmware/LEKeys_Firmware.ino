#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "led_output.h"
#include "pins.h"

int playback_speed_bpm = 60; // Playback speed in beats per minute
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
MD_MIDIFile SMF2; // Second MIDI file object that is going to point to the same file but 5 seconds in the future

// Timing Notes
// There are multiple things
// ticks - simply counts the number of ticks since the last event
// ticksPerQuarterNote - the number of ticks per quarter note, which is used to calculate the tempo
// timeSignature - the time signature of the MIDI file, e.g. 3/4 which is used to calculate the number of ticks per beat
// My LED have to foreshadow the next note and light it up when the note is played
// To do foreshadow the next note, I need to know the current note and the next note
// The MIDI file is played in a loop, so I need to know the current position in the file but also the next position in the future.
// Specifically a sensible future could be a full beat e.g. 4/4 in the future.
// My idea is to use the SMF2 object to point to the same file but one beat step in the future.

// The files in the tune list should be located on the SD card 
// or an error will occur opening the file and the next in the 
// list will be opened (skips errors).
const char *loopfile = "wonderwall.mid";  // simple and short file

int max_files = 10;
char **file_list = NULL; // Pointer to an array of file names

void fillFileListFromSD() {
  Serial.println("Filling file list from SD...");
  file_list = new char*[max_files]; // Allocate memory for 10 files
  //make sure all pointers are initialized to NULL
  for (int i = 0; i < max_files; i++) {
    file_list[i] = NULL;
  }

  int count = 0;
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("Found: ");
    if (file.isDirectory()) {
      Serial.println("Skipping directory.");
      file.close();
      file = root.openNextFile();
      continue;
    }
    char filename[21];
    file.getName(filename, 20);
    filename[20] = '\0';
    // Only add files ending with ".mid" (case-insensitive)
    int len = strlen(filename);
    if (len >= 4 && 
        (strcasecmp(filename + len - 4, ".mid") == 0)) {
      if (count < max_files) {
        char *stored = new char[len + 1];
        strcpy(stored, filename);
        file_list[count] = stored;
        Serial.print("Added to list: ");
        Serial.println(file_list[count]);
        count++;
      } else {
        Serial.println("Max file count reached, skipping remaining files.");
      }
    } else {
      Serial.println("Not a .mid file, skipping.");
    }
    file.close();
    file = root.openNextFile();
  }
  file_list[count] = NULL; // Null-terminate the list
  Serial.print("Total .mid files added: ");
  Serial.println(count);
}

void midiCallback2(midi_event *pev) {
    Serial.print("MIDI Event 2: ");
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
    strip.light(midi_note); // Light the note
    Serial.print("Lit note: ");
    Serial.println(midi_note);
  } else if (pev->data[0] == 0x80) { // Note Off event
    uint8_t midi_note = pev->data[1]; // MIDI note number
    strip.clear(midi_note); // Clear the strip for this note
    Serial.print("Cleared note: ");
    Serial.println(midi_note);
  }
  Serial.println();
}

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
    Serial.print("Lit note: ");
    Serial.println(midi_note);
  } else if (pev->data[0] == 0x80) { // Note Off event
    
  }
  Serial.println();
}

void main_button_isr(){
  buttonInput.handleInterrupt();
}

void setupPlayer(){
 // Initialize MIDIFile
    SMF.begin(&SD);
    SMF2.begin(&SD);
    SMF.setMidiHandler(midiCallback);
    SMF2.setMidiHandler(midiCallback2);
    SMF.looping(true);
    SMF2.looping(true);

    int err;
    err = SMF.load(loopfile);
  if (err != MD_MIDIFile::E_OK)
  {
    Serial.println("SMF load Error ");
    
    while (true);
  }

    err = SMF2.load(loopfile);
  if (err != MD_MIDIFile::E_OK)
  {
    Serial.println("SMF2 load Error "); 
    while (true);
  }
    Serial.print("SMF loaded: ");
    Serial.println(loopfile);
}

void setupLeds() {
    strip.begin(); // Initialize the LED strip
    strip.runTestSequence(); // Run the test sequence to check if the LEDs are working
    strip.setMainColor(strip.Color(0, 0, 255)); // Set main color to blue
    strip.setForeshadowColor(strip.Color(127, 0, 0)); // Set foreshadow color to dim red
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Keyboard");
    delay(1000);
    setupLeds(); // Setup the LED strip
    Wire.begin();
    ui.begin(); // Initialize the user interface
    
    #define SPI_SPEED SD_SCK_MHZ(4) //Does not work without that
    if (!SD.begin(SD_CS_PIN, SPI_SPEED))
    {
        Serial.println("\nSD init fail!");
        while (true) ;
    }

    fillFileListFromSD(); // Fill the file list from the SD card
    setupPlayer(); // Setup the MIDI player

    encoder.begin();
    buttonInput.begin(main_button_isr);

    Serial.println("Ready");
}

unsigned long tick_length = 100;
unsigned long last_tick = 0;

unsigned long current_tick = 0;
int last_bpm = 0; // Last BPM value to avoid unnecessary updates
bool rewinded = false; // Flag to indicate if the file has been rewound

void runStateMachine();

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
        SMF2.setTempo(playback_speed_bpm); // Update the tempo in the second MIDI file
    }
  /*if (!SMF.isEOF())
  {
    SMF.getNextEvent();
  }*/
  if (!SMF2.isEOF())
  {
    SMF2.getNextEvent();
  }

  if(!rewinded && millis() > 500){
    SMF2.restart(); // Rewind the second MIDI file
    rewinded = true; // Set the flag to true to avoid rewinding again
    Serial.println("MIDI file rewound");
  }

 ui.update(); // Update the user interface
// Remove encoder.process() from loop and use a timer interrupt instead
}


enum class DeviceState {
    INITIALIZING,
    PAUSED,
    PLAYING
};

DeviceState currentDeviceState = DeviceState::INITIALIZING;

void runStateMachine() {
    switch (currentDeviceState) {
    case DeviceState::INITIALIZING:
        if (millis() - last_tick > 1000) { // Update every second
            ui.showInitializing();
            last_tick = millis();
            currentDeviceState = DeviceState::PAUSED; // Move to paused state after initializing
        }
        break;
    case DeviceState::PAUSED:
        
        break;
    case DeviceState::PLAYING:
        if (millis() - last_tick > 200) { // Update every 200 milliseconds
            SMF.getNextEvent(); // Process the next MIDI event
            ui.showPlaying(playback_speed_bpm, SMF.getCurrentBar(), SMF.getMaxBars(), SMF.getCurrentBeat(), SMF.getLoopStart(), SMF.getLoopLength());
            last_tick = millis();
        }
        break;
    }
}