#include <SdFat.h>
#include <MD_MIDIFile.h>

#include "led_output.h"
#include "display_output.h"

#include "pins.h"

#include "encoder_input.h"
#include "button_input.h"

int playback_speed_bpm = 60; // Playback speed in beats per minute
int choosen_tune = 0; // Index of the currently chosen tune

int bpm = 120; // Beats per minute
int beatsPerBar = 4; // Beats per measure
int currentBeat = 0; // Current beat in the measure
int currentBar = 0; // Current bar in the song
int maxBars = 120; // Total number of bars in the song
int loopLength = 3; // Length of the loop in bars
int loopStart = 4;


enum class DeviceState {
    INITIALIZING1,
    INITIALIZING2,
    PAUSED,
    PLAYING
};
DeviceState currentDeviceState = DeviceState::INITIALIZING1;
unsigned long lastStateChange = 0;

Encoder_Input encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN);
Button_Input buttonInput(ENCODER_SW_PIN, 20); // Debounce delay 50ms
LedStrip strip(LED_PIN); // Create a LedStrip object with 61 LEDs (5 octaves + 1 for the highest note)
Display_Output display;

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

// Function Prototypes
void runStateMachine();
void switchState(DeviceState newState);
void fillFileListFromSD();
void setupPlayer();
void midiCallback(midi_event *pev);
void midiCallback2(midi_event *pev);
void main_button_isr();
void setupLeds();

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Keyboard");
    delay(1000);
    
    display.begin(); // Initialize the user interface
    switchState(DeviceState::INITIALIZING1); // Set initial state to INITIALIZING1
    runStateMachine(); // Run the state machine to update the display

    setupLeds(); // Setup the LED strip
    
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

    display.begin();
    display.update(); // Initial display update

    Serial.println("Ready");
}

unsigned long tick_length = 100;
unsigned long last_tick = 0;

unsigned long current_tick = 0;
int last_bpm = 0; // Last BPM value to avoid unnecessary updates
bool rewinded = false; // Flag to indicate if the file has been rewound

void loop() {
     handleBpmChange(); // Handle BPM changes

    if (!SMF2.isEOF())
    {
      SMF2.getNextEvent();
    }

    if(!rewinded && millis() > 500){
      SMF2.restart(); // Rewind the second MIDI file
      rewinded = true; // Set the flag to true to avoid rewinding again
      Serial.println("MIDI file rewound");
    }
    runStateMachine(); // Run the state machine to update the display and handle events
// Remove encoder.process() from loop and use a timer interrupt instead
}

////// STATE MACHINE FUNCTIONS //////
void runStateMachine() {
    switch (currentDeviceState) {
    case DeviceState::INITIALIZING1:
        if (millis() - lastStateChange > 1000) { // Update every second
            switchState(DeviceState::INITIALIZING2); // Switch to INITIALIZING2 after 1 second
        }
        break;
    case DeviceState::INITIALIZING2:
        if (millis() - lastStateChange > 1000) { // Update every second
            switchState(DeviceState::PLAYING); // Switch to PAUSED after 1 second
        }
        break;
    case DeviceState::PAUSED:
        
        break;
    case DeviceState::PLAYING:
        if (millis() - last_tick > 200) { // Update every 200 milliseconds
            SMF.getNextEvent(); // Process the next MIDI event
            display.showPlaying(bpm, (char *)loopfile, true, currentBar, maxBars, currentBeat, loopStart, loopLength);
            last_tick = millis();
        }
        break;
    }
}

void switchState(DeviceState newState) {
  Serial.print("Switching state from ");
  Serial.println(static_cast<int>(currentDeviceState));
  Serial.print(" to ");
  Serial.println(static_cast<int>(newState));

  currentDeviceState = newState;
  lastStateChange = millis(); // Reset the timer when switching states

  switch (newState) {
    case DeviceState::INITIALIZING1:
      display.showInitializing(1); // Show initializing screen 1
      break;
    case DeviceState::INITIALIZING2:
      display.showInitializing(0); // Show initializing screen 2
      break;
    case DeviceState::PAUSED:
      break;
    case DeviceState::PLAYING:
      display.showPlaying(bpm, (char *)loopfile, true, currentBar, maxBars, currentBeat, loopStart, loopLength); // Show playing screen
      break;
  }
}

////// SETUP FUNCTIONS //////
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

/////// INPUT HANDLING FUNCTIONS //////
void handleBpmChange() {
    if(last_bpm != playback_speed_bpm) {
        last_bpm = playback_speed_bpm; // Update last BPM
        tick_length = 60000 / playback_speed_bpm; // Calculate new tick length based on BPM
        Serial.print("Tick length updated to: ");
        Serial.println(tick_length);
        SMF.setTempo(playback_speed_bpm); // Update the tempo in the MIDI file
        SMF2.setTempo(playback_speed_bpm); // Update the tempo in the second MIDI file
    }
}

////// MIDI CALLBACK FUNCTIONS //////
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