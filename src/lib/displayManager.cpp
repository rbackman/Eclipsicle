#ifdef DISPLAY_MANAGER
#include "displayManager.h"

#define MISO_PIN -1 // Automatically assigned with ESP8266 if not defined
// #define MOSI_PIN 25 // Automatically assigned with ESP8266 if not defined
// #define SCLK_PIN 26 // Automatically assigned with ESP8266 if not defined

// #define CS_PIN 27  // Chip select control pin D8
// #define DC_PIN 32  // Data Command control pin
// #define RST_PIN 33 // Reset pin (could connect to NodeMCU RST, see next line)
// #define BL_PIN 13  // Backlight control pin (optional, can be connected to GPIO)
void DisplayManager::begin(int DC_PIN, int CS_PIN, int SCLK_PIN, int MOSI_PIN,
                           int RST_PIN, int BL_PIN, SPIClass *spi,
                           int miso_pin)

{
    Arduino_DataBus *bus = new Arduino_HWSPI(
        DC_PIN,   // DC
        CS_PIN,   // CS
        SCLK_PIN, // SCK
        MOSI_PIN, // MOSI
        miso_pin, // MISO pin (optional)
        spi,      // shared SPI bus
        true      // is_shared_interface
    );
    gfx = new Arduino_ST7796(
        bus, RST_PIN, 0, true, 320, 480, 0, 0);
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
    canvas = new GFXcanvas16(gfx->width(), gfx->height());
    if (canvas)
    {
        canvas->fillScreen(0x0000);
    }
    Serial.println("Display initialized");
}
void DisplayManager::showBars(const int *values, int len, int x, int y, int w, int h, uint16_t color)
{
    if (!canvas)
        return;

    canvas->drawRect(x, y, w, h, 0xFFFF); // Draw border
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        canvas->fillRect(x + i * barWidth, y, barWidth, h, 0x0000); // Clear bar area
        int barHeight = map(values[i], 0, 1023, 0, h);
        canvas->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color);
    }
    flush();
}
void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b)
{
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6)
    {
    case 0:
        r = v * 255;
        g = t * 255;
        b = p * 255;
        break;
    case 1:
        r = q * 255;
        g = v * 255;
        b = p * 255;
        break;
    case 2:
        r = p * 255;
        g = v * 255;
        b = t * 255;
        break;
    case 3:
        r = p * 255;
        g = q * 255;
        b = v * 255;
        break;
    case 4:
        r = t * 255;
        g = p * 255;
        b = v * 255;
        break;
    case 5:
        r = v * 255;
        g = p * 255;
        b = q * 255;
        break;
    }
}
void DisplayManager::drawBar(int index, int x, int y, int w, float h, int totalHeight, float hue)
{
    if (index < 0 || index >= 5)
    {
        Serial.println("Index out of bounds for drawBar");
        return;
    }

    if (!canvas)
        return;

    int barWidth = w;
    int barHeight = totalHeight * h; // height from 0 to totalHeight
    uint8_t r, g, b;
    hsvToRgb(hue, 1.0, 1.0, r, g, b); // hue: 0.0–1.0, full saturation and value
    uint16_t color = gfx->color565(r, g, b);

    int barX = x + index * barWidth;
    int barY = y - barHeight; // top of the bar

    // Clear the area first
    canvas->fillRect(barX, y - totalHeight, barWidth, totalHeight, 0x0000);

    // Draw the bar going upwards
    canvas->fillRect(barX + 1, barY, barWidth - 2, barHeight, color);
    flush();
}
void DisplayManager::showText(const String &text, int x, int y, int size, uint16_t color)
{
    if (!canvas)
        return;

    canvas->setTextSize(size);
    canvas->setTextColor(color, 0xFFFF);
    canvas->setCursor(x, y);
    canvas->print(text);
    flush();
}

void DisplayManager::showGraph(float *data, int len, int x, int y, int w, int h)
{
    if (!canvas)
        return;
    canvas->drawRect(x, y, w, h, 0xFFFF);
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        canvas->drawLine(x0, y0, x1, y1, 0xFFFF);
    }
    flush();
}

void DisplayManager::showParticles()
{
    if (!canvas)
        return;
    for (int i = 0; i < 50; i++)
    {
        int x = random(canvas->width());
        int y = random(canvas->height());
        uint16_t color = gfx->color565(random(255), random(255), random(255));
        canvas->fillCircle(x, y, 2, color);
    }
    flush();
}

void DisplayManager::clear()
{
    if (canvas)
    {
        canvas->fillScreen(0x0000);
        flush();
    }
}

void DisplayManager::flush()
{
    if (canvas && gfx)
    {
        gfx->draw16bitRGBBitmap(0, 0,
                                 (uint16_t *)canvas->getBuffer(),
                                 canvas->width(), canvas->height());
    }
}

#endif