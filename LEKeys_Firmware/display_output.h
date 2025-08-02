#ifndef DISPLAY_OUTPUT_H
#define DISPLAY_OUTPUT_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "bitmaps.h" // Include your bitmap definitions

class Display_Output
{
public:
    static constexpr int SCREEN_WIDTH = 128;
    static constexpr int SCREEN_HEIGHT = 64;
    static constexpr uint8_t OLED_ADDR = 0x3C;
    static constexpr unsigned long switchInterval = 3000; // 3 seconds
    static constexpr int numScreens = 4;

    Display_Output()
        : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
          lastSwitch(0),
          currentScreen(0)
    {
    }

    void begin()
    {
        Wire.begin();
        if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
        {
            for (;;)
                ; // Don't proceed, loop forever
        }
        display.clearDisplay();
        display.display();
    }

    void showInitializing(int bitmapArrayPosition)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.drawBitmap(0, 0, bitmap_allArray[bitmapArrayPosition], SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        display.display();
    }

    #define MAX_FILENAME_CHARS 14
    char screenStringBuffer[MAX_FILENAME_CHARS+1]; // MAX_FILENAME_CHARS + null terminator
    int scrollPosition = MAX_FILENAME_CHARS + 1; // Start before the first character
    unsigned long lastTextUpdate = 0;
    
    void drawFilename(const char *filename)
    {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(12, 2);

        int filenameLen = strlen(filename);
        if (filenameLen > MAX_FILENAME_CHARS)
        {
            scrollString((char *)filename, screenStringBuffer, MAX_FILENAME_CHARS);
            display.print(screenStringBuffer);
        }
        else
        {
            display.print(filename); // Display full filename
        }
    }

    void scrollString(char *toBeScrolled, char *outputBuffer, int maxChars)
    {
        int testStringLen = strlen(toBeScrolled);
        int totalScrollLen = testStringLen + maxChars;

        if (scrollPosition >= totalScrollLen)
        {
            scrollPosition = 0; // Reset scroll position after spaces and string
        }

        for (int i = 0; i < maxChars; i++)
        {
            int idx = scrollPosition + i;

            if ((idx > maxChars) & (idx <= testStringLen + maxChars))
            {
                outputBuffer[i] = toBeScrolled[idx - maxChars - 1];
            }
            else
            {
                outputBuffer[i] = ' '; // Fill with spaces
            }
        }
        outputBuffer[maxChars] = '\0'; // Null-terminate the string

        scrollPosition++; // Move to the next position for the next call
    }

    void showPlaying(int bpm, char *currentFilename, bool isPlaying, int currentBar, int maxBars, int currentBeat, int loopStart, int loopLength)
    {
        display.clearDisplay();
        drawFilename(currentFilename);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(105, 14);
        display.print(bpm);
        display.setCursor(105, 24);
        display.print("BPM");

        drawMetronome(currentBeat);
        drawPlayButton(isPlaying); // Draw play button
        draw_bar_boxes(currentBar, maxBars, loopStart, loopLength);
        display.display();
    }

    void drawPlayButton(bool isPlaying)
    {
        if (isPlaying)
        {
            // Draw Trangle Play Button in the upper left corner
            display.fillTriangle(0, 0, 6, 5, 0, 10, SSD1306_WHITE); // Draw filled triangle for play button
        }
        else
        {
            // Draw two rectangles for pause button
            display.fillRect(0, 0, 4, 10, SSD1306_WHITE); // Left rectangle
            display.fillRect(6, 0, 4, 10, SSD1306_WHITE); // Right rectangle
        }
    }

    void drawMetronome(int currentBeat)
    {
        if (currentBeat % 2 == 0)
        {
            display.drawCircle(115, 6, 5, SSD1306_WHITE); // Draw a circle for the beat
        }
        else
        {
            display.fillCircle(115, 6, 5, SSD1306_WHITE); // Fill circle for the beat
        }
    }

    void draw_bar_boxes(int currentBar, int maxBars, int loopStart, int loopLength = 1)
    {
        const int barsShown = 4;
        int pixelSpacing = 6;
        int boxWidth = (SCREEN_WIDTH / barsShown) - pixelSpacing;
        int boxHeight = 8;

        display.setTextSize(1);
        for (int i = 0; i < barsShown; i++)
        {
            display.setCursor(i * (boxWidth + pixelSpacing) + 10, SCREEN_HEIGHT - 20);
            display.print(currentBar + i + 1);

            int offset = pixelSpacing + i * boxWidth + i * pixelSpacing;

            if (i < loopLength)
            {
                display.fillRect(offset, SCREEN_HEIGHT - 11, boxWidth, boxHeight, SSD1306_WHITE);
            }
            else
            {
                display.drawRect(offset, SCREEN_HEIGHT - 11, boxWidth, boxHeight, SSD1306_WHITE);
            }

            if (i == 0)
            {
                display.drawLine(offset, SCREEN_HEIGHT - 1, offset + boxWidth - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE); // Draw line for current bar
            }
        }
    }

    void update()
    {
        unsigned long now = millis();
        if (now - lastSwitch > switchInterval)
        {
            currentScreen = (currentScreen + 1) % numScreens;
            lastSwitch = now;
        }

        display.clearDisplay();
        display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, SSD1306_WHITE);

        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(30, 28);

        switch (currentScreen)
        {
        case 0:
            // Example bitmap array (replace with your own image)
            static const unsigned char PROGMEM logo_bmp[] = {
                0xFF, 0x81, 0xBD, 0xA5, 0xA5, 0xBD, 0x81, 0xFF};
            // Draw bitmap at (40, 20), 8x8 pixels
            display.drawBitmap(40, 20, logo_bmp, 8, 8, SSD1306_WHITE);
            display.setCursor(60, 28);
            display.print("screen 1");
            break;
        case 1:
            display.drawBitmap(0, 0, bitmap_allArray[0], SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
            break;
        case 2:
            display.drawBitmap(0, 0, bitmap_allArray[1], SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        case 3:
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