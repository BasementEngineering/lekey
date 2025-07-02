#pragma once

#include <Adafruit_SSD1306.h>

#include <CtrlEnc.h>
#define SCREEN_ADDRESS 0x3C

// UserInterface class definition
class UserInterface {
public:
    // Constructor
    UserInterface(Adafruit_SSD1306* display,
                  int* bpm//,
                  //Callback playCallback,
                  //Callback pauseCallback
                  )
        : display_(display),
          bpm_(bpm),
          //playCallback_(playCallback),
          //pauseCallback_(pauseCallback),
          state_(State::Paused),
          currentScreen_(Screen::PlayingScreen),
          midiTrack_(0),
          currentTimeMs_(0),
          lastButtonPressTime_(0),
          buttonPressCount_(0),
          buttonHeld_(false),
          encoderTurnedWhileHeld_(false)
    {
    }

    void begin() {
        display_->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
        display_->clearDisplay();
        display_->setTextSize(1);
        display_->setTextColor(SSD1306_WHITE);
        display_->display();
    }

    void onTurnLeft() {
        if (bpm_ && *bpm_ > 20) {
            (*bpm_)--;
        }
    }

    void onTurnRight() {
        if (bpm_) {
            (*bpm_)++;
        }
    }

    void update(){
        switch (state_) {
            case State::Playing:
                showPlayingScreen();
                break;
            case State::Paused:
                showPausedScreen();
                break;
        }
    }

private:
    // Internal state
    enum class State {
        Playing,
        Paused
    };

    // Menu screens
    enum class Screen {
        PlayingScreen,
        // Add more screens as needed
    };

    // Hardware
    Adafruit_SSD1306* display_;
    CtrlEnc* encoder_;
    int* bpm_;

    // State
    State state_;
    Screen currentScreen_;
    uint8_t midiTrack_;
    uint32_t currentTimeMs_;

    // Encoder button handling
    uint32_t lastButtonPressTime_;
    uint8_t buttonPressCount_;
    bool buttonHeld_;
    bool encoderTurnedWhileHeld_;

    void showPlayingScreen() {
        display_->clearDisplay();
        display_->setTextSize(1);
        display_->setTextColor(SSD1306_WHITE);
        display_->setCursor(0, 0);
        display_->print("Playing  BPM:");
        display_->print(bpm_ ? *bpm_ : 0);

        display_->setCursor(0, 16);
        display_->print("Track: ");
        display_->print(midiTrack_);

        display_->setCursor(0, 32);
        display_->print("Time: ");
        display_->print((unsigned long)currentTimeMs_);
        display_->print(" ms");

        display_->display();
    }

    void showPausedScreen() {
        display_->clearDisplay();
        display_->setTextSize(1);
        display_->setTextColor(SSD1306_WHITE);
        display_->setCursor(0, 0);
        display_->print("Paused   BPM:");
        display_->print(bpm_ ? *bpm_ : 0);

        display_->setCursor(0, 16);
        display_->print("Track: ");
        display_->print(midiTrack_);

        display_->setCursor(0, 32);
        display_->print("Time: ");
        display_->print((unsigned long)currentTimeMs_);
        display_->print(" ms");

        display_->display();
    }

    void rewind() {
        if (currentTimeMs_ > 1000) currentTimeMs_ -= 1000;
        else currentTimeMs_ = 0;
    }

    void forward() {
        currentTimeMs_ += 1000;
    }
};

