#ifdef LED_TEST
#include <FastLED.h>
#define NUM_LEDS 16
CRGB ledsStrip[NUM_LEDS];
#define MASTER_LED_PIN 33
int count = 0;
void setup()
{
    FastLED.addLeds<NEOPIXEL, MASTER_LED_PIN>(ledsStrip, NUM_LEDS);
    FastLED.setBrightness(50);
    Serial.begin(115200);
}

void loop()
{
    count++;
    for (int i = 0; i < NUM_LEDS; i++)
    {

        ledsStrip[i] = CHSV((i + count) * 255 / NUM_LEDS, 255, 255);
    }
    FastLED.show();
    delay(100);
}

#endif