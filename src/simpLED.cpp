

#ifdef SIMP_LED

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "leds.h"
#include "sensors.h"
#include "serial.h"
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
  SensorManager *sensorManager = new SensorManager(grid);
 LEDManager *ledManager = new LEDManager(33, 144);
SerialManager *serialManager = new SerialManager(80);

void setup()
{

    // serialManager = new SerialManager(80);

    // sensorManager = new SensorManager(grid);

    // ledManager = new LEDManager(33, 144, sensorManager);

    // ledManager->setBrightness(20);
    ledManager->set
}

void loop()
{
    serialManager->updateSerial();
    //  sensorManager->updateSensors();
    ledManager->update();
    int rangefinderValue = sensorManager->getSensorValue(RANGEFINDER, 0, 1000);
    //  turn on a different number of leds based on sensor value

}

#endif