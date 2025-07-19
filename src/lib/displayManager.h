#pragma once
#ifdef DISPLAY_MANAGER
#include <Arduino_GFX_Library.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#ifndef DISPLAY_USE_DOUBLE_BUFFER
#define DISPLAY_USE_DOUBLE_BUFFER 1
#endif

class DisplayManager
{
    int _DC_PIN;
    int _CS_PIN;
    int _RST_PIN;
    int _BL_PIN;

public:
    void showText(const String &text, int x, int y, int size = 2, uint8_t color = 0xFF);
    void showGraph(float *data, int len, int x, int y, int w, int h);
    void showParticles();
    void clear();
    void begin(int DC_PIN, int CS_PIN, int SCLK_PIN, int MOSI_PIN,
               int RST_PIN, int BL_PIN, SPIClass *spi = &SPI,
               int MISO_PIN = -1);
    void showBars(const int *values, int len, int x, int y, int w, int h, uint8_t color = 0xFF);
    void drawBar(int index, int x, int y, int w, float h, int totalHeight, float hue);

private:
    Arduino_GFX *gfx;
#if DISPLAY_USE_DOUBLE_BUFFER
    GFXcanvas8 *canvas = nullptr;
    uint16_t color332To565(uint8_t c);
    uint8_t rgbTo332(uint8_t r, uint8_t g, uint8_t b);
#endif
    void flush();
};
#endif
