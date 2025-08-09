

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
    void connectSlaves(std::vector<MacAddress> slaves);
    void sendSensorCommandToSlaves(sensor_message msg);
    void sendImageToSlaves(image_message image);
    void sendParametersToSlaves(ParameterID paramID, int value);
    void sendStringToSlaves(String msg);
    void init();

    void setImageHandler(ImageHandler handler) { _imageHandler = handler; }
    void setTextHandler(TextHandler handler) { _textHandler = handler; }
    void setSensorHandler(SensorHandler handler) { _sensorHandler = handler; }
    void setParameterHandler(ParameterHandler handler) { _parameterHandler = handler; }
    std::string getMacAddress();

private:
    ImageHandler _imageHandler = nullptr;
    TextHandler _textHandler = nullptr;
    SensorHandler _sensorHandler = nullptr;
    ParameterHandler _parameterHandler = nullptr;

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

private:
    static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) 
    {

        uint8_t messageType = *data;
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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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

            memcpy(&msg, data, sizeof(msg));

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
