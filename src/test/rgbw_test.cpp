#ifdef RGBW_TEST
#include <FastLED.h>

#define NUM_LEDS 10

#define DATA_PIN 4

CRGB leds[NUM_LEDS];

int LED_INDEX = 0;
Rgbw rgbw = Rgbw(
    kRGBWDefaultColorTemp,
    kRGBWNullWhitePixel, // Mode
    W0                   // W-placement
);

// typedef WS2811<DATA_PIN, RGB> ControllerT;                     // RGB mode must be RGB, no re-ordering allowed.
// static RGBWEmulatedController<ControllerT, BRG> rgbwEmu(rgbw); // ordering goes here.

void setup()
{
    Serial.begin(115200);
    // FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS).setRgbw(RgbwDefault());
    // FastLED.addLeds(&rgbwEmu, leds, NUM_LEDS); //.setRgbw(RgbwDefault());

    //      Color Order test {
    // check each color order and see the results..  number in brackets is which LED lights up
    // NO_COLOR //  g[0] b[0] r[1]
    // 	RGB=0012,  /// g[0] b[0] r[1]
    // 	RBG=0021,  /// g[0] r[1] b[0]
    // 	GRB=0102,   ///<  b[0] g[0] r[1]
    // 	GBR=0120,  /// r[1]  g[0] b[0]
    // 	BRG=0201,  /// b[0] r[1] g[0]
    // 	BGR=0210   /// r[1] b[0] g[0]
    // };

    FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.setBrightness(128); // Set global brightness to 50%
    delay(2000);                // If something ever goes wrong this delay will allow upload.
}

// Cycle r,g,b,w. Red will blink once, green twice, ... white 4 times.
void loop()
{

    leds[LED_INDEX] = CRGB::Red;
    FastLED.show();
    delay(1000);

    leds[LED_INDEX] = CRGB::Green;
    FastLED.show();
    delay(1000);

    leds[LED_INDEX] = CRGB::Blue;
    FastLED.show();
    delay(1000);

    leds[LED_INDEX] = CRGB::Black;
    FastLED.show();
    delay(1000);
}

#endif