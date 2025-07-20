#ifdef DISPLAY_MANAGER
#include "displayManager.h"
#include "pins.h"
#define MISO_PIN -1 // Automatically assigned with ESP8266 if not defined
// #define MOSI_PIN 25 // Automatically assigned with ESP8266 if not defined
// #define SCLK_PIN 26 // Automatically assigned with ESP8266 if not defined

// #define CS_PIN 27  // Chip select control pin D8
// #define DC_PIN 32  // Data Command control pin
// #define RST_PIN 33 // Reset pin (could connect to NodeMCU RST, see next line)
// #define BL_PIN 13  // Backlight control pin (optional, can be connected to GPIO)

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

uint8_t DisplayManager::rgbTo332(uint8_t r, uint8_t g, uint8_t b)
{
    return (r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6);
}

void DisplayManager::begin(SPIClass *spi)
{

    Arduino_DataBus *bus = new Arduino_HWSPI(
        DISPLAY_DC,   // DC
        DISPLAY_CS,   // CS
        DISPLAY_SCL,  // SCK
        DISPLAY_MOSI, // MOSI
        DISPLAY_MISO, // MISO pin (optional)
        spi,          // shared SPI bus
        true          // is_shared_interface
    );
    gfx = new Arduino_ST7796(
        bus, DISPLAY_RST, 0, true, 320, 480, 0, 0);
    if (!gfx->begin())
    {
        Serial.println("Failed to initialize display");
        return;
    }
    gfx->fillScreen(0x0000); // Clear screen with black color
    pinMode(DISPLAY_BL, OUTPUT);
    digitalWrite(DISPLAY_BL, HIGH); // Turn on backlight
    gfx->setRotation(1);            // landscape
    gfx->fillScreen(0x0000);
#if DISPLAY_USE_DOUBLE_BUFFER
    canvas = new GFXcanvas8(gfx->width(), gfx->height());
    if (canvas)
    {
        canvas->fillScreen(0x00);
        flush();
    }
    else
    {
        Serial.println("Failed to allocate display buffer");
    }
#endif
    Serial.println("Display initialized");
}
void DisplayManager::showBars(const int *values, int len, int x, int y, int w, int h, uint8_t color)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;

    canvas->drawRect(x, y, w, h, 0xFF); // Draw border
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        int barHeight = map(values[i], 0, 1023, 0, h);
        // Clear area above the bar
        canvas->fillRect(x + i * barWidth, y, barWidth, h - barHeight, 0x00);
        // Draw the bar
        canvas->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color);
    }
    flush();
#else
    gfx->drawRect(x, y, w, h, 0xFF);
    int barWidth = w / len;
    for (int i = 0; i < len; i++)
    {
        int barHeight = map(values[i], 0, 1023, 0, h);
        gfx->fillRect(x + i * barWidth, y, barWidth, h - barHeight, 0x00);
        gfx->fillRect(x + i * barWidth + 1, y + h - barHeight, barWidth - 2, barHeight, color332To565(color));
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
    uint8_t color = rgbTo332(r, g, b);

    int barX = x + index * barWidth;
    int barY = y - barHeight; // top of the bar

    // Clear only the area above the bar
    if (totalHeight > barHeight)
        canvas->fillRect(barX, y - totalHeight, barWidth, totalHeight - barHeight, 0x00);

    // Draw the bar going upwards
    canvas->fillRect(barX + 1, barY, barWidth - 2, barHeight, color);
    flush();
#else
    int barWidth = w;
    int barHeight = totalHeight * h;
    uint8_t r, g, b;
    hsvToRgb(hue, 1.0, 1.0, r, g, b);
    uint8_t color = rgbTo332(r, g, b);

    int barX = x + index * barWidth;
    int barY = y - barHeight;

    gfx->fillRect(barX, y - totalHeight - barY, barWidth, totalHeight, 0x0000);
    gfx->fillRect(barX + 1, barY, barWidth - 2, barHeight, color332To565(color));

#endif
}

void DisplayManager::showText(const std::string &text, int x, int y, int size, uint8_t color)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;

    canvas->setTextSize(size);
    canvas->setTextColor(color, 0xFF);
    canvas->setCursor(x, y);
    canvas->print(text);
    flush();
#else
    gfx->setTextSize(size);
    gfx->setTextColor(color332To565(color), color332To565(0xFF));
    gfx->setCursor(x, y);
    gfx->print(text.c_str());
#endif
}

void DisplayManager::showGraph(float *data, int len, int x, int y, int w, int h)
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (!canvas)
        return;
    canvas->drawRect(x, y, w, h, 0xFF);
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        canvas->drawLine(x0, y0, x1, y1, 0xFF);
    }
    flush();
#else
    gfx->drawRect(x, y, w, h, color332To565(0xFF));
    for (int i = 1; i < len; i++)
    {
        int x0 = x + (i - 1) * w / len;
        int x1 = x + i * w / len;
        int y0 = y + h - (data[i - 1] * h);
        int y1 = y + h - (data[i] * h);
        gfx->drawLine(x0, y0, x1, y1, color332To565(0xFF));
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
        uint8_t color = rgbTo332(random(255), random(255), random(255));
        canvas->fillCircle(x, y, 2, color);
    }
    flush();
#else
    int w = gfx->width();
    int h = gfx->height();
    for (int i = 0; i < 50; i++)
    {
        int x = random(w);
        int y = random(h);
        uint8_t color = rgbTo332(random(255), random(255), random(255));
        gfx->fillCircle(x, y, 2, color332To565(color));
    }
#endif
}

void DisplayManager::clear()
{
#if DISPLAY_USE_DOUBLE_BUFFER
    if (canvas)
    {
        canvas->fillScreen(color332To565(0x00));
        flush();
    }
#else
    gfx->fillScreen(0x00);
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
            gfx->draw16bitBeRGBBitmap(0, y, line, w, 1);
        }
    }
#endif
}

#endif