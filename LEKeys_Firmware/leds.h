#ifndef LEDS_H
#define LEDS_H

#include <Adafruit_NeoPixel.h>

class LedStrip {
public:
    LedStrip(uint8_t pin,uint16_t numLeds=(5*12+1))
        : strip(numLeds, pin, NEO_GRB + NEO_KHZ800), numLeds(numLeds) {
    }

    void begin() {
        Serial.println("Initializing LED strip...");
        strip.begin();
        strip.show();
        foreshadowColor = strip.Color(127, 127, 127); // Default: white at 50%
        main_color = strip.Color(255, 255, 255);      // Default: white at 100%
    }

    void setForeshadowColor(uint32_t color) {
        foreshadowColor = color;
    }

    void setMainColor(uint32_t color) {
        main_color = color;
    }

    void foreshadow(uint8_t midi_note) {
        uint16_t ledIndex = midiNoteToLedIdx(midi_note);
        uint32_t color50 = dimColor(foreshadowColor, 0.5f);
        strip.setPixelColor(ledIndex, color50);
        strip.show();
    }

    void light(uint8_t midi_note) {
        uint16_t ledIndex = midiNoteToLedIdx(midi_note);
        strip.setPixelColor(ledIndex, main_color);
        strip.show();
    }

    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return strip.Color(r, g, b);
    }

    void clear() {
        strip.clear();
        strip.show();
    }

private:
    Adafruit_NeoPixel strip;
    uint16_t numLeds;
    uint32_t foreshadowColor;
    uint32_t main_color;


    // Midi Note lookup table
    

    uint16_t midiNoteToLedIdx(uint8_t midi_note) {
        Serial.print("midi_note: ");
        Serial.println(midi_note);
        const int min_keyboard_octave = 1; // Octaves start at -1
        const int max_keyboard_octave = 4; // Octaves end at 4

        int note = midi_note % 12; // Get the note within the octave (0-11)
        int octave = int(midi_note / 12)-1; // Get the octave (octaves start at -1)

        if( octave < min_keyboard_octave || octave > max_keyboard_octave ){
            Serial.println("Octave out of range");
            return 0; // Out of range, return first LED
        }

        int ledIndex = (octave - min_keyboard_octave) * 12 + note; // Map to LED index

        Serial.print("ledIndex: ");
        Serial.println(ledIndex);
        return ledIndex;
    }

    uint32_t dimColor(uint32_t color, float factor) {
        uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * factor);
        uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * factor);
        uint8_t b = (uint8_t)((color & 0xFF) * factor);
        return strip.Color(r, g, b);
    }
};

#endif // LEDS_H