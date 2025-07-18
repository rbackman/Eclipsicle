#ifdef DISPLAY_MANAGER
#include "displayManager.h"

#define MISO_PIN -1 // Automatically assigned with ESP8266 if not defined
// #define MOSI_PIN 25 // Automatically assigned with ESP8266 if not defined
// #define SCLK_PIN 26 // Automatically assigned with ESP8266 if not defined

// #define CS_PIN 27  // Chip select control pin D8
// #define DC_PIN 32  // Data Command control pin
// #define RST_PIN 33 // Reset pin (could connect to NodeMCU RST, see next line)
// #define BL_PIN 13  // Backlight control pin (optional, can be connected to GPIO)
void DisplayManager::beginSPI(int DC_PIN, int CS_PIN, int SCLK_PIN, int MOSI_PIN, int RST_PIN, int BL_PIN, int spiHost)

{
    Serial.println("Initializing display...");
    Arduino_DataBus *bus = new Arduino_ESP32SPI(
        DC_PIN,   // DC
        CS_PIN,   // CS
        SCLK_PIN, // SCK
        MOSI_PIN, // MOSI
        -1,       // MISO (not used)
        spiHost   // SPI bus
    );
    gfx = new Arduino_ST7796(
        bus, RST_PIN, 0, true, 320, 480, 0, 80);
    if (!gfx->begin())
    {
        Serial.println("Failed to initialize display");
        return;
    }
    gfx->fillScreen(0x0000); // Clear screen with black color
    pinMode(BL_PIN, OUTPUT);
    digitalWrite(BL_PIN, HIGH); // Turn on backlight
    gfx->setRotation(1);        // landscape
    gfx->fillScreen(0x0000);
    Serial.println("Display initialized");
}
void DisplayManager::showBars(const int *values, int len, int x, int y, int w, int h, uint16_t color)
{
    gfx->drawRect(x, y, w, h, color);
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        int barHeight = map(values[i], 0, 1023, 0, h);
        gfx->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color);
    }
}
void DisplayManager::showText(const String &text, int x, int y, int size, uint16_t color)
{
    // clear the previous text

    gfx->setTextSize(size);
    gfx->setTextColor(color, 0xFFFF);
    gfx->setCursor(x, y);
    gfx->print(text);
}

void DisplayManager::showGraph(float *data, int len, int x, int y, int w, int h)
{
    gfx->drawRect(x, y, w, h, 0xFFFF);
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        gfx->drawLine(x0, y0, x1, y1, 0xFFFF);
    }
}

void DisplayManager::showParticles()
{
    for (int i = 0; i < 50; i++)
    {
        int x = random(gfx->width());
        int y = random(gfx->height());
        uint16_t color = gfx->color565(random(255), random(255), random(255));
        gfx->fillCircle(x, y, 2, color);
    }
}

void DisplayManager::clear()
{
    gfx->fillScreen(0x0000);
}

#endif