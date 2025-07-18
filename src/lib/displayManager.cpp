#ifdef DISPLAY_MANAGER
#include "displayManager.h"

#define MISO_PIN -1 // Automatically assigned with ESP8266 if not defined
// #define MOSI_PIN 25 // Automatically assigned with ESP8266 if not defined
// #define SCLK_PIN 26 // Automatically assigned with ESP8266 if not defined

// #define CS_PIN 27  // Chip select control pin D8
// #define DC_PIN 32  // Data Command control pin
// #define RST_PIN 33 // Reset pin (could connect to NodeMCU RST, see next line)
// #define BL_PIN 13  // Backlight control pin (optional, can be connected to GPIO)

#if DISPLAY_USE_DOUBLE_BUFFER
uint16_t DisplayManager::color332To565(uint8_t c)
{
    uint8_t r = (c >> 5) & 0x07;
    uint8_t g = (c >> 2) & 0x07;
    uint8_t b = c & 0x03;
    uint16_t r5 = (r * 31) / 7;
    uint16_t g6 = (g * 63) / 7;
    uint16_t b5 = (b * 31) / 3;
    return (r5 << 11) | (g6 << 5) | b5;
}

uint8_t DisplayManager::color565To332(uint16_t c)
{
    uint8_t r5 = (c >> 11) & 0x1F;
    uint8_t g6 = (c >> 5) & 0x3F;
    uint8_t b5 = c & 0x1F;
    uint8_t r = (r5 * 7) / 31;
    uint8_t g = (g6 * 7) / 63;
    uint8_t b = (b5 * 3) / 31;
    return (r << 5) | (g << 2) | b;
}
#endif
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
#if DISPLAY_USE_DOUBLE_BUFFER
    canvas = new GFXcanvas8(gfx->width(), gfx->height());
    if (canvas)
    {
        canvas->fillScreen(color565To332(0x0000));
        flush();
    }
    else
    {
        Serial.println("Failed to allocate display buffer");
    }
#endif
    Serial.println("Display initialized");
}
void DisplayManager::showBars(const int *values, int len, int x, int y, int w, int h, uint16_t color)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;

    canvas->drawRect(x, y, w, h, color565To332(0xFFFF)); // Draw border
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        canvas->fillRect(x + i * barWidth, y, barWidth, h, color565To332(0x0000)); // Clear bar area
        int barHeight = map(values[i], 0, 1023, 0, h);
        canvas->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color565To332(color));
    }
    flush();
#else
    gfx->drawRect(x, y, w, h, color);
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        gfx->fillRect(x + i * barWidth, y, barWidth, h, 0x0000);
        int barHeight = map(values[i], 0, 1023, 0, h);
        gfx->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color);
    }
#endif
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
#if DISPLAY_USE_DOUBLE_BUFFER
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
    canvas->fillRect(barX, y - totalHeight, barWidth, totalHeight, color565To332(0x0000));

    // Draw the bar going upwards
    canvas->fillRect(barX + 1, barY, barWidth - 2, barHeight, color565To332(color));
    flush();
#else
    int barWidth = w;
    int barHeight = totalHeight * h;
    uint8_t r, g, b;
    hsvToRgb(hue, 1.0, 1.0, r, g, b);
    uint16_t color = gfx->color565(r, g, b);

    int barX = x + index * barWidth;
    int barY = y - barHeight;

    gfx->fillRect(barX, y - totalHeight, barWidth, totalHeight, 0x0000);
    gfx->fillRect(barX + 1, barY, barWidth - 2, barHeight, color);
#endif
}
void DisplayManager::showText(const String &text, int x, int y, int size, uint16_t color)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;

    canvas->setTextSize(size);
    canvas->setTextColor(color565To332(color), color565To332(0xFFFF));
    canvas->setCursor(x, y);
    canvas->print(text);
    flush();
#else
    gfx->setTextSize(size);
    gfx->setTextColor(color, 0xFFFF);
    gfx->setCursor(x, y);
    gfx->print(text);
#endif
}

void DisplayManager::showGraph(float *data, int len, int x, int y, int w, int h)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;
    canvas->drawRect(x, y, w, h, color565To332(0xFFFF));
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        canvas->drawLine(x0, y0, x1, y1, color565To332(0xFFFF));
    }
    flush();
#else
    gfx->drawRect(x, y, w, h, 0xFFFF);
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        gfx->drawLine(x0, y0, x1, y1, 0xFFFF);
    }
#endif
}

void DisplayManager::showParticles()
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;
    for (int i = 0; i < 50; i++)
    {
        int x = random(canvas->width());
        int y = random(canvas->height());
        uint16_t color = gfx->color565(random(255), random(255), random(255));
        canvas->fillCircle(x, y, 2, color565To332(color));
    }
    flush();
#else
    int w = gfx->width();
    int h = gfx->height();
    for (int i = 0; i < 50; i++)
    {
        int x = random(w);
        int y = random(h);
        uint16_t color = gfx->color565(random(255), random(255), random(255));
        gfx->fillCircle(x, y, 2, color);
    }
#endif
}

void DisplayManager::clear()
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (canvas)
    {
        canvas->fillScreen(color565To332(0x0000));
        flush();
    }
#else
    gfx->fillScreen(0x0000);
#endif
}

void DisplayManager::flush()
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (canvas && gfx)
    {
        uint16_t w = canvas->width();
        uint16_t h = canvas->height();
        static uint16_t *line = nullptr;
        static uint16_t line_w = 0;
        if (line_w < w)
        {
            if (line)
                free(line);
            line = (uint16_t *)malloc(sizeof(uint16_t) * w);
            line_w = w;
        }
        uint8_t *buf = canvas->getBuffer();
        for (uint16_t y = 0; y < h; y++)
        {
            for (uint16_t x = 0; x < w; x++)
            {
                uint8_t c = buf[y * w + x];
                line[x] = color332To565(c);
            }
            gfx->drawRGBBitmap(0, y, line, w, 1);
        }
    }
#endif
}

#endif