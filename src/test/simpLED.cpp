

#ifdef SIMP_LED
#define FASTLED_RMT5_RECYCLE 1
#include <Arduino.h>
#include <FastLED.h>
#include "ledManager.h"

LEDManager *ledManager;
void setup()
{

    Serial.begin(115200);
    Serial.println("Starting up... esp32 LED test");
    ledManager = new LEDManager("simpled");
}

void loop()
{

    ledManager->update();
    // delay(50);
}

#endif