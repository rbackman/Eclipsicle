#pragma once
#ifdef DISPLAY_MANAGER
#include <Arduino_GFX_Library.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

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
    uint16_t color332To565(uint8_t c);
    uint8_t rgbTo332(uint8_t r, uint8_t g, uint8_t b);
#if DISPLAY_USE_DOUBLE_BUFFER
    GFXcanvas8 *canvas = nullptr;

#endif
    void flush();
};
#endif
