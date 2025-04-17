

#ifdef SIMP_LED

#include <Arduino.h>
#include "leds.h"
#include "sensors.h"

#include "ledManager.h"
//  Adafruit_NeoPixel mstrip = Adafruit_NeoPixel(144, 13, NEO_GRB + NEO_KHZ800);
led color;

SensorGrid grid = {
    // {SLIDER, 36, "Slider1"},
    // {SLIDER, 39, "Slider2"},
    // {SLIDER, 34, "Slider3"},
    {IR_RANGEFINDER, 35, RANGEFINDER},
    // {SLIDER, 35, "Slider4"},
    // {SLIDER, 32, "Slider5"},
    // {BUTTON, 26, "Button1"},
    // {BUTTON, 27, "Button2"},
    // {BUTTON, 14, "Button3"},
    // {BUTTON, 12, "Button4"},
};
// SensorManager *sensorManager = new SensorManager(grid);

LEDManager *ledManager;
// SerialManager *serialManager;
void setup()
{
    std::string name;
    std::vector<StripState *> strips = {
        new StripState(LED_STATE_RAINBOW, 80, 0, false),

    };
    ledManager = new LEDManager("LEDManager", strips);
    // serialManager = new SerialManager(80); // Initialize the serialManager

    // sensorManager = new SensorManager(grid);

    // ledManager = new LEDManager(33, 144, sensorManager);

    // ledManager->setBrightness(20);
    // ledManager->setAll({0, 0, 255});
    // color.r = 255;
    // color.g = 0;
    // color.b = 0;
    // ledManager->setLED(3, color);
    ledManager->setBrightness(20);
}

void loop()
{

    //  sensorManager->updateSensors();
    ledManager->update();
    // int rangefinderValue = sensorManager->getSensorValue(RANGEFINDER, 0, 1000);
    // Serial.println("Rangefinder Value: " + String(rangefinderValue));
    //  turn on a different number of leds based on sensor value
}

#endif