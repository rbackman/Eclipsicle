#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>

class SerialManager
{
private:
    char *buffer;
    int bufPos;
    bool _stringAvailable;
    bool _jsonAvailable;
    int bufferSize;

    bool echo = true;

public:
    SerialManager(int bufferSize = 1024);
    ~SerialManager(); // You may want a destructor if you are going to deallocate the buffer

    void clearBuffer();

    void updateSerial();
    bool stringAvailable();
    bool jsonAvailable();
    String readString();
    char *readBuffer();
    bool readJson(JsonDocument &doc);
};
