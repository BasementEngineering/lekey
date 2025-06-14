#ifndef DISPLAY_OUTPUT_H
#define DISPLAY_OUTPUT_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display_Output {
public:
    static constexpr int SCREEN_WIDTH = 128;
    static constexpr int SCREEN_HEIGHT = 64;
    static constexpr uint8_t OLED_ADDR = 0x3C;
    static constexpr unsigned long switchInterval = 3000; // 3 seconds
    static constexpr int numScreens = 3;

    Display_Output()
        : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
          lastSwitch(0),
          currentScreen(0)
    {}

    void begin() {
        Wire.begin();
        if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
            for (;;); // Don't proceed, loop forever
        }
        display.clearDisplay();
        display.display();
    }

    void update() {
        unsigned long now = millis();
        if (now - lastSwitch > switchInterval) {
            currentScreen = (currentScreen + 1) % numScreens;
            lastSwitch = now;
        }

        display.clearDisplay();
        display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, SSD1306_WHITE);

        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(30, 28);

        switch (currentScreen) {
            case 0:
                display.print("screen 1");
                break;
            case 1:
                display.print("screen 2");
                break;
            case 2:
                display.print("screen 3");
                break;
        }

        display.display();
    }

private:
    Adafruit_SSD1306 display;
    unsigned long lastSwitch;
    int currentScreen;
};

#endif // DISPLAY_OUTPUT_H