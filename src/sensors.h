#pragma once
#include "shared.h"

// void initSensors(SensorGrid &sensors);
// void updateSensor();
// int getSensorValue(String name, int min, int max);

// bool handleSensorCommand(String command);

#define DEBOUNCE_DELAY 50

class SensorManager
{
private:
    int printSensor = -1; // -2 = print all sensors, -1 = print none

    int currentSensor = 0;
    static SensorManager *instance;
    SensorGrid sensorGrid;

public:
    SensorManager(SensorGrid sensors);

    void updateSensors();
    int getSensorValue(SensorID name, int min, int max);
    bool buttonPressed(SensorID name);
    bool handleSensorCommand(String command);
    bool messageAvailable();
    sensor_message getNextMessage();

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
