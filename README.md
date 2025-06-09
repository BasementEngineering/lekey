# LEKeys
LEKeys or "LED Keys" is a little project that is supposed to help me with learning the piano.
It illuminates the key that has to be played next, with the ability to adjust the playback speed and mark certain sections that i want to practice.
![alt text](docs/images/logo_white.png)
## Hardware
-Wemo D1 Mini
- WS2812B LED strip
- OLED display
- 3D printed case
- SD card reader

## Features
- Put MIDI Songs on micro SD card
- Change placback Speed
- OLED 

## Pin Issues
- Rotary Encoder with push button requires 3 pins
- Possible Option is to connect it to a single Analog Input: https://www.elektormagazine.com/labs/rotary-encoder-on-a-single-mcu-pin
using an R-2R resistor network

## Controls
Control Sheme: Single Rotary Encoder with Push Button

Main Interactions
- Play/Pause
- Change Playback Speed
- Change MIDI Song
- Rewind and Forward

IOs
- Rotary Encoder: 2 Pins (16,0)
- Encoder push button: Analog Pin (A0)

Control actions:
- Play/Pause: Press Encoder Button
- Change Playback Speed: Rotate Encoder
- Change MIDI Song: Double Press Encoder Button
- Rewind and Forward: Rotate Encoder while pressing button

## Aquiring MIDI Songs
- Use a MIDI Editor to create your own MIDI Songs
- Use a mp3 to MIDI converter e.g. [Transkun](https://github.com/Yujia-Yan/Transkun/tree/main)
To run it, you use the following command
```bash
transkun mp3_files/wonderwall.mp3 output.mid
```

However, so far, Transukun does not work. Installation via pip worked, but the program states 
"FileNotFoundError: [WinError 2] The system cannot find the file specified"
Although, the file is there. Entering an incorrect path throws a different error, stating the missing filename.