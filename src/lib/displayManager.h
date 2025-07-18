#pragma once
#ifdef DISPLAY_MANAGER
#include <Arduino_GFX_Library.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

class DisplayManager
{
    int _DC_PIN;
    int _CS_PIN;
    int _RST_PIN;
    int _BL_PIN;

public:
    void showText(const String &text, int x, int y, int size = 2, uint16_t color = 0xFFFF);
    void showGraph(float *data, int len, int x, int y, int w, int h);
    void showParticles();
    void clear();
    void begin(int DC_PIN, int CS_PIN, int SCLK_PIN, int MOSI_PIN,
               int RST_PIN, int BL_PIN, SPIClass *spi = &SPI,
               int MISO_PIN = -1);
    void showBars(const int *values, int len, int x, int y, int w, int h, uint16_t color = 0xFFFF);
    void drawBar(int index, int x, int y, int w, float h, int totalHeight, float hue);

private:
    Arduino_GFX *gfx;
    GFXcanvas8 *canvas = nullptr;
    uint16_t color332To565(uint8_t c);
    uint8_t color565To332(uint16_t c);
    void flush();
};
#endif
