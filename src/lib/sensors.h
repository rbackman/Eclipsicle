#pragma once
#ifdef USE_SENSORS

#include "shared.h"
#include <SPI.h>
#define DEBOUNCE_DELAY 10

class SensorManager
{
private:
    int printSensor = -1; // -2 = print all sensors, -1 = print none

    int currentSensor = 0;
    static SensorManager *instance;
    SensorGrid sensorGrid;
    SPIClass *spiBus;

public:
    SensorManager(SensorGrid sensors, SPIClass *spi = nullptr);

    void updateSensors();
    int getSensorValue(SensorID name, int min, int max);
    bool buttonPressed(SensorID name);
    bool handleSensorCommand(String command);
    bool messageAvailable();
    sensor_message getNextMessage();
    int readADC(SensorState *sensor);
    // void handleButtonPressed(String name)
    // {
    //     unsigned long currentTime = millis();
    //     for (int i = 0; i < grid.size(); i++)
    //     {
    //         if (grid[i].name == name && grid[i].value == 0)
    //         {
    //             if (currentTime - grid[i].lastDebounceTime > DEBOUNCE_DELAY)
    //             {
    //                 grid[i].value = 1;
    //                 grid[i].lastDebounceTime = currentTime;
    //             }
    //         }
    //     }
    // }
};

#endif