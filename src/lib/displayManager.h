#pragma once
#ifdef DISPLAY_MANAGER
#include <Arduino_GFX_Library.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "shared.h"

struct ParameterDisplayItem
{
    std::string name;       // parameter label
    float normalized;       // value from 0.0 to 1.0
    std::string valueText;  // value string for display
};

// #ifndef DISPLAY_USE_DOUBLE_BUFFER
// #define DISPLAY_USE_DOUBLE_BUFFER 1
// #endif

#ifdef USE_DISPLAY
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
{ // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
        ; // Don't proceed, loop forever
}
display.display();
delay(500); // Pause for 2 seconds
display.setTextSize(1);
display.setTextColor(WHITE);
display.setRotation(0);

Serial.println("Adafruit MPU6050 found display!");
#endif

class DisplayManager
{
    int _DC_PIN;
    int _CS_PIN;
    int _RST_PIN;
    int _BL_PIN;

public:
    void showText(const std::string &text, int x, int y, int size = 2, uint8_t color = 0xFF);
    void showGraph(float *data, int len, int x, int y, int w, int h);
    void showParticles();
    void clear();
    void begin(SPIClass *spi = &SPI);
    void showBars(const int *values, int len, int x, int y, int w, int h, uint8_t color = 0xFF);
    void drawBar(int index, int x, int y, int w, float newNorm, float oldNorm, int totalHeight);
    void displayMenu(const std::vector<std::string> &menuItems, int selectedIndex = -1)
    {
        clear();
        for (size_t i = 0; i < menuItems.size(); ++i)
        {
            bool highlight = (static_cast<int>(i) == selectedIndex);
            int y = 10 + i * 20;
            if (highlight)
            {
#if DISPLAY_USE_DOUBLE_BUFFER
                if (canvas)
                {
                    canvas->fillRect(0, y - 2, gfx->width(), 18, 0xFF);
                }
#else
                gfx->fillRect(0, y - 2, gfx->width(), 18, color332To565(0xFF));
#endif
            }
            uint8_t textColor = highlight ? 0x00 : 0xFF;
            showText(menuItems[i], 10, y, 2, textColor);
        }
    }
    void displayParameterBars(const std::vector<ParameterDisplayItem> &items, int selectedIndex = -1,
                              const std::string &header = "", bool forceClear = false);

private:
    Arduino_GFX *gfx;
    uint16_t color332To565(uint8_t c);
    uint8_t rgbTo332(uint8_t r, uint8_t g, uint8_t b);
#if DISPLAY_USE_DOUBLE_BUFFER
    GFXcanvas8 *canvas = nullptr;

#endif
    void flush();
    std::vector<ParameterDisplayItem> lastParams;
    int lastSelected = -1;
    std::string lastHeader;
    bool firstDraw = true;
};
#endif
