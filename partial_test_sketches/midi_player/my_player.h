#ifndef MY_PLAYER_H
#define MY_PLAYER_H

#include <SdFat.h>
#include <MD_MIDIFile.h>
#include "pins.h"
#include <Arduino.h>

class MyPlayer {
public:
    static MyPlayer& getInstance() {
        static MyPlayer instance;
        return instance;
    }

    // Static callback wrapper
    static void playerCallbackStatic(midi_event *pev) {
        getInstance().playerCallback(pev);
    }

    void begin() {
        #define SPI_SPEED SD_SCK_MHZ(4)
        if (!SD.begin(SD_CS_PIN, SPI_SPEED)) {
            Serial.println("SD card initialization failed!");
            while (true);
            return;
        }
        Serial.println("SD card initialized successfully.");
        fillFileListFromSD();
        SMF.begin(&SD);
        SMF2.begin(&SD);
        SMF.setMidiHandler(playerCallbackStatic); // Use static callback

        int beats_per_bar = (SMF.getTimeSignature() >> 8) & 0xFF;
        horizonLengthInTicks = horizonLengthInBars * beats_per_bar * SMF.getTicksPerQuarterNote(); // Set horizon length to 4 bars
    }

    void setNoteBuffer(uint8_t note, bool state)
    {
        // Update the note buffer to keep track of currently pressed notes
        if (note < 128)
        {
            noteBuffer[note] = state;
           /* if (state)
            {
                Serial.print("Note ON: ");
                Serial.println(note);
            }
            else
            {
                Serial.print("Note OFF: ");
                Serial.println(note);
            }*/
        }
    }

    void printNoteBuffer()
    {
        // Print the current state of the note buffer
        Serial.println("Current Note Buffer:");
        for (int i = 0; i < 128; i++)
        {
            if (noteBuffer[i])
            {
                Serial.print("x");
            }
            Serial.print(" ");
        }
        Serial.println();
    }

    void playerCallback(midi_event *pev)
    {
        // Handle MIDI events here
        // This function will be called for each MIDI event
        Serial.print("MIDI Event: ");
        Serial.print("Track: ");
        Serial.println(pev->track);
        Serial.print("Channel: ");
        Serial.println(pev->channel);

        if (pev->data[0] == 0x90) { // Note On event
            uint8_t midi_note = pev->data[1]; // MIDI note number
           setNoteBuffer(midi_note, true);
           printNoteBuffer();
        } else if (pev->data[0] == 0x80) { // Note Off event
            setNoteBuffer(pev->data[1], false); // MIDI note number
            printNoteBuffer();
        }
    }

    void play()
    {
        // Start playing the MIDI file
        //SMF.play();
        //SMF2.play();
        Serial.println("Playing MIDI file");
    }

    void pause()
    {
        // Pause the MIDI file
        //SMF.pause();
        //SMF2.pause();
        Serial.println("Paused MIDI file");
    }

    void nextSong()
    {
        // Move to the next song in the list
        choosen_song++;
        if (choosen_song >= max_files || file_list[choosen_song] == NULL)
        {
            Serial.println("No more songs in the list.");
            choosen_song = max_files - 1; // Reset to last song
            return;
        }

        Serial.print("Next song: ");
        Serial.println(file_list[choosen_song]);
        SMF.load(file_list[choosen_song]);
        SMF.looping(true); // Enable looping for the first MIDI file
        //SMF2.load(file_list[choosen_song]);
        Serial.println("Loaded next song.");
    }

    void previousSong()
    {
        // Move to the previous song in the list
        choosen_song--;
        if (choosen_song < 0)
        {
            Serial.println("Already at the first song.");
            choosen_song = 0; // Reset to first song
            return;
        }

        Serial.print("Previous song: ");
        Serial.println(file_list[choosen_song]);
        SMF.load(file_list[choosen_song]);
        SMF2.load(file_list[choosen_song]);
        Serial.println("Loaded previous song.");
    }

    char *getChoosenSong()
    {
        return file_list[choosen_song];
    }

    int getBpm()
    {
        // Get the current BPM from the MIDI file
        return SMF.getTempo();
    }

    void slower()
    {
        // Decrease the BPM by 10
        int current_bpm = SMF.getTempo();
        current_bpm -= 10;
        if (current_bpm < 30)
            current_bpm = 30; // Minimum BPM limit
        SMF.setTempo(current_bpm);
        SMF2.setTempo(current_bpm);
        Serial.print("BPM decreased to: ");
        Serial.println(current_bpm);
    }

    void faster()
    {
        // Increase the BPM by 10
        int current_bpm = SMF.getTempo();
        current_bpm += 10;
        if (current_bpm > 300)
            current_bpm = 300; // Maximum BPM limit
        SMF.setTempo(current_bpm);
        SMF2.setTempo(current_bpm);
        Serial.print("BPM increased to: ");
        Serial.println(current_bpm);
    }

    int getCurrentBar()
    {
        // Get the current bar and beat from the MIDI file
        unsigned long tick_duration_us = SMF.getTickTime(); // Get the tick duration in milliseconds
        unsigned long ticks_per_quarter_note = SMF.getTicksPerQuarterNote(); // Get the ticks per quarter note
        
        unsigned long current_tick = lastTick;
        int current_beat = current_tick / ticks_per_quarter_note; // Calculate the current beat
        int beats_per_bar = (SMF.getTimeSignature() >> 8) & 0xFF;
        int current_bar = current_beat / beats_per_bar; // Calculate the current bar
    
        /*
        Serial.print("Beats per bar: ");
        Serial.println(beats_per_bar);
        Serial.print("Tick duration: ");
        Serial.print(tick_duration_us);
        Serial.println(" us");
        Serial.print("Ticks per quarter note: ");
        Serial.println(ticks_per_quarter_note);
        */

        return current_bar;
    }

    void run()
    {
        if (!SMF.isEOF())
        {
            SMF.getNextEvent();
        }
    }

    /*void setMidiHandler(void (*callback)(midi_event *pev))
    {
        // Set the MIDI callback function
        SMF.setMidiHandler(callback);
        SMF2.setMidiHandler(callback);
    }*/

    

    void update()
    {
        if (!SMF.isEOF())
        {
            //Serial.println("Updating MIDI player...");
            SMF.getNextEvent();
        }

        uint16_t ticks = tickClock(); // Update the MIDI file's tick clock
        if(ticks == 0){
            return;
        }
        lastTick += ticks;

       /* Serial.print("Tick: ");
        Serial.println(ticks);
        
        Serial.print("Last Tick: ");
        Serial.println(lastTick);
        */

        // Process the MIDI events
        
    }

private:
    MyPlayer() : SD(), SMF(), SMF2() {}
    ~MyPlayer() {}
    MyPlayer(const MyPlayer&) = delete;
    MyPlayer& operator=(const MyPlayer&) = delete;
    MyPlayer(MyPlayer&&) = delete;
    MyPlayer& operator=(MyPlayer&&) = delete;
    
    SdFat SD;
    MD_MIDIFile SMF;
    MD_MIDIFile SMF2; // Second MIDI file object that is going to point to the same file but 5 seconds in the future

    int max_files = 10;
    char **file_list = NULL; // Pointer to an array of file names
    int choosen_song = -1;   // Index of the currently chosen tune

    bool noteBuffer[128] = {false}; // Buffer to keep track of currently pressed notes

    uint16_t tickClock(void)
    // check if enough time has passed for a MIDI tick and work out how many!
    {
        uint32_t  elapsedTime;
        uint16_t  ticks = 0;

        elapsedTime = _lastTickError + micros() - _lastTickCheckTime;
        unsigned long _tickTime = SMF.getTickTime(); // Get the tick time in microseconds
        if (elapsedTime >= _tickTime)
        {
            ticks = elapsedTime/_tickTime;
            _lastTickError = elapsedTime - (_tickTime * ticks);
            _lastTickCheckTime = micros();    // save for next round of checks
        }

        return(ticks);
    }

    void fillFileListFromSD()
    {
        Serial.println("Filling file list from SD...");
        file_list = new char *[max_files]; // Allocate memory for 10 files
        // make sure all pointers are initialized to NULL
        for (int i = 0; i < max_files; i++)
        {
            file_list[i] = NULL;
        }

        int count = 0;
        File root = SD.open("/");
        File file = root.openNextFile();
        while (file)
        {
            Serial.print("Found: ");
            if (file.isDirectory())
            {
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
                (strcasecmp(filename + len - 4, ".mid") == 0))
            {
                if (count < max_files)
                {
                    char *stored = new char[len + 1];
                    strcpy(stored, filename);
                    file_list[count] = stored;
                    Serial.print("Added to list: ");
                    Serial.println(file_list[count]);
                    count++;
                }
                else
                {
                    Serial.println("Max file count reached, skipping remaining files.");
                }
            }
            else
            {
                Serial.println("Not a .mid file, skipping.");
            }
            file.close();
            file = root.openNextFile();
        }
        file_list[count] = NULL; // Null-terminate the list

        if (count > 0)
        {
            choosen_song = 0; // Set the first song as the default choice
            Serial.print("Default song set to: ");
            Serial.println(file_list[choosen_song]);
        }
        else
        {
            Serial.println("No .mid files found on SD card.");
        }

        Serial.print("Total .mid files added: ");
        Serial.println(count);
    }


/*
typedef struct
{
  uint8_t track;    ///< the track this was on
  uint8_t channel;  ///< the midi channel
  uint8_t size;     ///< the number of data bytes
  uint8_t data[4];  ///< the data. Only 'size' bytes are valid
} midi_event;
*/

typedef struct
{
  uint8_t size;     ///< the number of data bytes
  uint8_t data[4];  ///< the data. Only 'size' bytes are valid
  uint16_t duration;
} led_midi_event;

/* 
toickClock() in midiFile can be used to check if a tick has passed
It will always return 0 if you are in between ticks, but as soon as a tick_time hat passed
it will return the current tick number.
*/

    midi_event event_buffer[1024]; //FiFo buffer for MIDI events
    int event_buffer_index = 0; // Index for the event buffer
    uint16_t lastTick = 0;
    unsigned long _lastTickError = 0; // Error in the last tick calculation
    unsigned long _lastTickCheckTime = 0; // Last time the tick was checked

    int horizonLengthInTicks = -1;
    int horizonLengthInBars = 4; // Length of the horizon in bars, used to determine how many ticks to look ahead
};

#endif // MY_PLAYER_H