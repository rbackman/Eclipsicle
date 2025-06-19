

#include <WiFi.h>

#include "shared.h"
#include <ArduinoJson.h>
#include "serial.h"

void SerialManager::clearBuffer()
{
    _stringAvailable = false;
    _jsonAvailable = false;
    memset(buffer, 0, bufferSize);

    bufPos = 0;
}
SerialManager::~SerialManager()
{
    free(buffer);
}
SerialManager::SerialManager(int size, String name)
{
    _name = name;
    bufferSize = size;

    buffer = (char *)malloc(sizeof(char) * bufferSize);
    bufPos = 0; // Initialize bufPos

    if (buffer == nullptr)
    {
        Serial.println("Error allocating buffer");
        return;
    }

    memset(buffer, 0, bufferSize);
    Serial.begin(115200);

    // Serial.println("ESPNow Init Success  " + String(WiFi.macAddress()));
    Serial.printf("Starting serial on device %s  \nwith address %s /n", getName(), String(WiFi.macAddress()).c_str());
}
void SerialManager::updateSerial()
{
    try
    {
        if (Serial.available() > 0)
        {

            // look for some serial input from the user and forward it to the slaves
            if (bufPos >= bufferSize - 1)
            {
                // If the buffer is full, reset it:
                Serial.println("WARNING! buffer full, bufPos=" + String(bufPos));
                clearBuffer();
            }

            char data = Serial.read();
            if (isVerbose())
            {
                Serial.print(data);
            }

            if (data == 27)
            {

                clearBuffer();
            }

            else if (data == ';' and bufPos > 0 and bufPos < bufferSize - 2)
            {

                buffer[bufPos] = '\0'; // Null-terminate the string

                _jsonAvailable = true;

                return;
            }
            else if (data == '\n' and bufPos > 0 and bufPos < bufferSize - 1)
            {

                if (isVerbose())
                {
                    Serial.println("Newline received");
                    Serial.println("buffer size " + String(bufPos));
                    Serial.printf("val: %s", buffer);
                }
                buffer[bufPos] = data;
                buffer[bufPos + 1] = '\0'; // Null-terminate the string

                String res = String(buffer);
                res.trim();

                if (res == "clear")
                {
                    clearBuffer();
                    return;
                }
                if (res == "verbose")
                {
                    setVerbose(!isVerbose());
                    Serial.println("verbose set to " + String(isVerbose()));
                    clearBuffer();

                    return;
                }
                if (res.indexOf("getMac") != -1)
                {
                    Serial.println("deviceid," + getName() + "," + WiFi.macAddress());
                    clearBuffer();
                    _stringAvailable = false;
                    return;
                }

                if (res.length() > 0)
                {
                    _stringAvailable = true;
                }
                else
                {
                    _stringAvailable = false;
                }
            }
            else if (bufPos >= bufferSize - 1)
            {
                // If the buffer is full, reset it:
                Serial.println("WARNING! buffer full reseting buffer bufPos=" + String(bufPos));
                Serial.println("buffer: " + String(buffer));
                clearBuffer();
            }
            else
            {
                // If the character is not a newline, add it to the buffer:
                buffer[bufPos] = data;
                bufPos++;
            }
        }
    }
    catch (const std::exception &e)
    {
        Serial.println("Error in buffer loop");
        // Serial.println(e.what());
        clearBuffer();
    }
}
bool SerialManager::stringAvailable()
{

    return _stringAvailable;
}
bool SerialManager::jsonAvailable()
{
    return _jsonAvailable;
}
String SerialManager::readString()
{
    _stringAvailable = false;
    String res = String(buffer);
    res.trim();
    clearBuffer();
    return res;
}

char *SerialManager::readBuffer()
{
    _stringAvailable = false;
    char *res = buffer;
    clearBuffer();
    return res;
}
bool SerialManager::readJson(JsonDocument &doc)
{
    _jsonAvailable = false;

    // Serial.println("Current Buffer as strinf:" + String(buffer));
    //  just use the first split of ; to decode json
    char *jsonString = strtok(buffer, ";");
    if (jsonString == nullptr)
    {
        Serial.println("Error: jsonString is null");
        clearBuffer();
        return false;
    }

    auto error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        Serial.println("Buffer: " + String(jsonString));
        clearBuffer();
        return false;
    }
    clearBuffer();
    // if (isVerbose())
    // {
    //     Serial.print("Json Deserialized: ");
    //     serializeJson(doc, Serial);
    //     Serial.println();
    // }

    return true;
}
