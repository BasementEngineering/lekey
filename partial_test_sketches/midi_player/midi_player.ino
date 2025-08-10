#include "my_player.h"

#include "pins.h"

MyPlayer* my_player = &MyPlayer::getInstance();

void midiCallback(midi_event *pev) {
/*
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
  */
}

int lastBar = 0;
void printPlayerStatus() {
    int currentBar = my_player->getCurrentBar();

    if(lastBar != currentBar){
      Serial.print("Current BPM: ");
      Serial.println(my_player->getBpm());
      Serial.print("Current song: ");
      Serial.println(my_player->getChoosenSong());
      Serial.print("Current Bar: ");
      Serial.println(my_player->getCurrentBar());
      Serial.println(); 
      lastBar = currentBar;
    }
    
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Player");
    delay(1000);
    
    my_player->begin();
    //my_player->setMidiHandler(midiCallback);
}

int counter = 0;
unsigned long lastPlayerEventTs = 0;

void loop() {
  //my_player->run();
  my_player->update();
  if( (millis() - lastPlayerEventTs) > 2000){
    lastPlayerEventTs = millis();
    counter++;
    counter%=4;
    switch(counter){
      case 0:
        my_player->play();
        Serial.println("Playing MIDI file");
        break;
      case 1:
        my_player->pause();
        Serial.println("Paused MIDI file");
        break;
      case 2:
        my_player->nextSong();
        Serial.println("Next song");
        break;
      case 3:
        my_player->previousSong();
        Serial.println("Previous song");
        break;
      default:
        Serial.println("Unknown action");
        break;
    }


  }
  //printPlayerStatus();
  
}