#ifndef KEYSTROKE_BUFFER_H
#define KEYSTROKE_BUFFER_H

#include <Arduino.h>

const size_t NUM_KEYS = 72;

struct Keystroke {
    uint8_t key;         // MIDI key number (0-71)
    uint32_t pressTime;  // Timestamp when key was pressed (millis)
    uint32_t releaseTime;// Timestamp when key was released (millis)
    bool active;         // Is the key currently pressed?
};

class KeystrokeBuffer {
public:
    KeystrokeBuffer();
    ~KeystrokeBuffer();

    void noteOn(uint8_t key);
    void noteOff(uint8_t key);
    bool getKeystroke(uint8_t key, Keystroke &ks) const;
    void clear();

private:
    Keystroke keystrokes[NUM_KEYS];
};

#endif // KEYSTROKE_BUFFER_H