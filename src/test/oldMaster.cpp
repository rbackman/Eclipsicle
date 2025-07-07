

#ifdef OLD_MASTER_BOARD

#include "meshnet.h"
#include "shared.h"
#include <vector>
#include "sensors.h"
#include "serial.h"
#include "motors.h"

#include "leds.h"

const std::vector<std::string> slaves = {"Go", "Slave1"};

SensorGrid sensors{
    {SLIDER, 36, 0, 0, {0, 0, 0, 0, 0}, "Slider1"},
    {SLIDER, 39, 0, 0, {0, 0, 0, 0, 0}, "Slider2"},
    {SLIDER, 34, 0, 0, {0, 0, 0, 0, 0}, "Slider3"},
    {SLIDER, 35, 0, 0, {0, 0, 0, 0, 0}, "Slider4"},
    {SLIDER, 32, 0, 0, {0, 0, 0, 0, 0}, "Slider5"},
};

void setup()
{

    initSerial(80);
    // initMesh();
    // connectSlaves(slaves);
    // initSensors(sensors);
    initLEDS(13, 144);

    // Serial.println("Starting Master with address: " + WiFi.macAddress());

    // initMotors();
}

void loop()
{

    if (stringAvailable())
    {

        String command = readString();

        // if (handleMotorCommand(command))
        // {
        //     Serial.println("Motor command handled");
        //     return;
        // }
        if (handleLEDCommand(command))
        {
            Serial.println("LED command handled");
        }
       
    }
    try
    {
        updateLEDs();
    }
    catch (const std::exception &e)
    {
        Serial.println("Error in updateLEDs");
        Serial.println(e.what());
    }
}

#endif