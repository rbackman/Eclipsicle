

#ifdef SIMP_LED

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "leds.h"
#include "sensors.h"
#include "serial.h"
//  Adafruit_NeoPixel mstrip = Adafruit_NeoPixel(144, 13, NEO_GRB + NEO_KHZ800);
led color;

SensorGrid grid = {
    // {SLIDER, 36, "Slider1"},
    // {SLIDER, 39, "Slider2"},
    // {SLIDER, 34, "Slider3"},
    // {SLIDER, 35, "Slider4"},
    // {SLIDER, 32, "Slider5"},
    // {BUTTON, 26, "Button1"},
    // {BUTTON, 27, "Button2"},
    // {BUTTON, 14, "Button3"},
    // {BUTTON, 12, "Button4"},
};
//  SensorManager *sensorManager = new SensorManager(grid);
 LEDManager *ledManager = new LEDManager(33, 144);
SerialManager *serialManager = new SerialManager(80);

void setup()
{

    // serialManager = new SerialManager(80);

    // sensorManager = new SensorManager(grid);

    // ledManager = new LEDManager(33, 144, sensorManager);

    // ledManager->setBrightness(20);
}

void loop()
{
    serialManager->updateSerial();
    //  sensorManager->updateSensors();
    ledManager->update();

    // if (sensorManager->buttonPressed("Button1"))
    // {
    //     ledManager->setAll({r : 255, g : 255, b : 255});
    //     Serial.println("Button 1 pressed");
    // }
    // if (sensorManager->buttonPressed("Button2"))
    // {
    //     ledManager->setAll({r : 255, g : 0, b : 0});
    //     Serial.println("Button 2 pressed");
    // }
    // if (sensorManager->buttonPressed("Button3"))
    // {
    //     ledManager->setAll({r : 0, g : 255, b : 0});
    //     Serial.println("Button 3 pressed");
    // }
    // if (sensorManager->buttonPressed("Button4"))
    // {
    //     ledManager->setAll({r : 0, g : 0, b : 255});
    //     Serial.println("Button 4 pressed");
    // }
    // if (button1)
    // {

    //     button1 = false;
    //     ledManager->setBrightness(255);
    //     ledManager->setAll({r : 255, g : 255, b : 255});
    // }
    // if (button2)
    // {

    //     button2 = false;
    //     ledManager->setBrightness(1);
    // }
    // if (button3)
    // {
    //     button3 = false;

    //     ledManager->setAll({r : 255, g : 0, b : 0});
    // }
    // if (button4)
    // {
    //     button4 = false;

    //     ledManager->setAll({r : 0, g : 255, b : 0});
    // }
}

#endif