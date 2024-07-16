

#pragma once
#include <string>
#include <vector>
#include "shared.h"
#include <functional>

#include <esp_now.h>

using ImageHandler = std::function<void(const image_message &)>;
using TextHandler = std::function<void(const text_message &)>;
using SensorHandler = std::function<void(const sensor_message &)>;
using ParameterHandler = std::function<void(const parameter_message &)>;

class MeshnetManager
{

    static MeshnetManager *instance; // For access in the callback
public:
    MeshnetManager();

public:
    void connectSlaves(std::vector<std::string> slaves);
    void sendSensorCommandToSlaves(sensor_message msg);
    void sendImageToSlaves(image_message image);
    void sendParametersToSlaves(ParameterID paramID, int value);
    void sendStringToSlaves(String msg);
    void init();

    void setImageHandler(ImageHandler handler) { _imageHandler = handler; }
    void setTextHandler(TextHandler handler) { _textHandler = handler; }
    void setSensorHandler(SensorHandler handler) { _sensorHandler = handler; }
    void setParameterHandler(ParameterHandler handler) { _parameterHandler = handler; }


private:
    ImageHandler _imageHandler = nullptr;
    TextHandler _textHandler = nullptr;
    SensorHandler _sensorHandler = nullptr;
    ParameterHandler _parameterHandler = nullptr;

    MacAddresses addresses = {
        {"Slave1", {0xEC, 0x94, 0xCB, 0x6A, 0xFE, 0x08}},       // slave1
        {"Go", {0xB0, 0xA7, 0x32, 0x81, 0x9A, 0x88}},           // light board "GO"
        {"Pine", {0xC8, 0xC9, 0xA3, 0xCF, 0xA1, 0xB4}},         // master  "pine"
        {"Slave3", {0xB0, 0xB2, 0x1C, 0xA8, 0x0C, 0x9C}},       // slave3
        {"SuperSpinner", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC}}, // 40:91:51:FB:F7:BC
        {                                                       // Eclipsicle  40:91:51:FB:B7:48
         "Eclipsicle",
         {0x40, 0x91, 0x51, 0xFB, 0xB7, 0x48}}

        // {0xA8, 0x42, 0xE3, 0xC8, 0x36, 0xC8}} // A8:42:E3:C8:36:C8
    };

    std::vector<MacAddress> _slaves;

    // Then make a regular member function to handle the received data
    void handleImageMessage(image_message msg)
    {
        if (_imageHandler)
        {
            _imageHandler(msg);
        }
        else
        {
            Serial.println("No image handler");
        }
    };
    void handleTextMessage(text_message msg)
    {
        if (_textHandler)
        {
            _textHandler(msg);
        }
        else
        {
            Serial.println("No text handler");
        }
    };
    void handleSensorMessage(sensor_message msg)
    {
        if (_sensorHandler)
        {
            _sensorHandler(msg);
        }
        else
        {
            Serial.println("No sensor handler");
        }
    };
    void handleMessageSent(esp_now_send_status_t status)
    {
        // if (status == ESP_NOW_SEND_SUCCESS)
        // {
        //     // Serial.println("Message sent successfully");
        // }
        // else
        // {
        //     Serial.println("Message failed to send");
        // }
    }
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
    {

        uint8_t messageType = *incomingData;
        // Serial.println("got espnow Message type: " + String(messageType));
        if (messageType == MESSAGE_TYPE_IMAGE)
        {

            image_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }
            // Serial.println("got image: " + String(msg.size));
            // Serial.println("Memory size before " + String(ESP.getFreeHeap()));

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleImageMessage(msg);
        }
        else if (messageType == MESSAGE_TYPE_TEXT)
        {
            text_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleTextMessage(msg);
        }
        else if (messageType == MESSAGE_TYPE_SENSOR)
        {
            sensor_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleSensorMessage(msg);
        }
        else if(messageType==MESSAGE_TYPE_PARAMETER){
            parameter_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            if (instance->_parameterHandler)
            {
                instance->_parameterHandler(msg);
            }
            else
            {
                Serial.println("No parameter handler");
            }
        }
        else if (messageType == MESSAGE_TYPE_TEXT)
        {
            text_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleTextMessage(msg);
        }
        else if (messageType == MESSAGE_TYPE_SENSOR)
        {
            sensor_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleSensorMessage(msg);
        }
        else if (messageType == MESSAGE_TYPE_PARAMETER)
        {
            parameter_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            if (instance->_parameterHandler)
            {
                instance->_parameterHandler(msg);
            }
            else
            {
                Serial.println("No parameter handler");
            }
        }
        else if (messageType == MESSAGE_TYPE_TEXT)
        {
            text_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleTextMessage(msg);
        }
        else if (messageType == MESSAGE_TYPE_SENSOR)
        {
            sensor_message msg;
            if (len != sizeof(msg))
            {
                Serial.println("Unexpected data length received." + String(len) + " vs " + String(sizeof(msg)));
                return;
            }

            memcpy(&msg, incomingData, sizeof(msg));

            instance->handleSensorMessage(msg);
        }
       
        else
        {
            Serial.println("Unknown message type " + String(messageType));
        }
    }
    static void OnDataSent(const uint8_t *mac, esp_now_send_status_t status)
    {
        instance->handleMessageSent(status);
    }
};
