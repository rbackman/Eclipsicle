#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>

class SerialManager
{
private:
    char *buffer = nullptr;
    int bufPos = 0;
    bool _stringAvailable = false;
    bool _jsonAvailable = false;
    int bufferSize = 0;
    bool echo = true;
    char *readBuffer();

public:
    SerialManager(int bufferSize = 1024);
    ~SerialManager(); // You may want a destructor if you are going to deallocate the buffer

    void clearBuffer();

    void updateSerial();
    bool stringAvailable();
    bool jsonAvailable();
    String readString();

    bool readJson(JsonDocument &doc);
};
