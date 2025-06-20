#ifdef MESH_NET

#include <esp_now.h>
#include <WiFi.h>
#include "shared.h"
#include <vector>
#include "meshnet.h"

const std::vector<LEDRig> slaves = {
    {
        "Eclipsicle",
        {0x40, 0x91, 0x51, 0xFB, 0xB7, 0x48},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},
            {200, 1, ANIMATION_TYPE_SLIDER, false},

        },
    },
    {
        "Tesseratica",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},

        },
    },
    {
        "tradeday",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {100, 0, ANIMATION_TYPE_IDLE, false},
        },
    },
    {
        "simpled",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},

        },
    },
    {
        "Bike",
        {0xD0, 0xEF, 0x76, 0x58, 0x45, 0xB4},
        {
            {48, 0, ANIMATION_TYPE_SLIDER, false},
            {48, 1, ANIMATION_TYPE_SLIDER, false},
            {18, 2, ANIMATION_TYPE_SLIDER, false},
        },
    },
    {

        "Spinner",
        {0xD0, 0xEF, 0x76, 0x58, 0x45, 0xB4},
        {
            {280, 0, ANIMATION_TYPE_IDLE, false},
            {280, 1, ANIMATION_TYPE_IDLE, false},
        },
    },

    {
        "Bike",
        {0xD0, 0xEF, 0x76, 0x57, 0x3F, 0xA0},
        {
            {100, 0, ANIMATION_TYPE_IDLE, false},
            {100, 1, ANIMATION_TYPE_IDLE, false},
            {100, 2, ANIMATION_TYPE_IDLE, false},
        },
    }};

void printMacAddress(const uint8_t *macAddress)
{
    char macStr[18];
    sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x",
            macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    Serial.println(macStr);
}

MeshnetManager *MeshnetManager::instance = nullptr;
//
MeshnetManager::MeshnetManager()
{
    instance = this;
    init();
}

void MeshnetManager::connectSlaves()

{

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(MeshnetManager::OnDataSent);
    // Register peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    for (int i = 0; i < slaves.size(); i++)
    {
        MacAddress mac = slaves[i].mac;
        _slaves.push_back(mac);

        memcpy(peerInfo.peer_addr, mac.data(), 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        // Add peer
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer");
            printMacAddress(peerInfo.peer_addr);
        }
        else
        {
            Serial.println("Connected to Slave Board with MAC Address: ");
            printMacAddress(peerInfo.peer_addr);
        }
    }
};

void MeshnetManager::init()
{

    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(MeshnetManager::OnDataRecv);
};

void MeshnetManager::sendStringToSlaves(String command)
{
    text_message msg;
    // Populate msg with your data
    msg.numBytes = command.length();
    msg.type = MESSAGE_TYPE_TEXT;
    strcpy(msg.text, command.c_str());

    for (int i = 0; i < _slaves.size(); i++)
    {
        esp_now_send(_slaves[i].data(), (uint8_t *)&msg, sizeof(msg));
    }
}
void MeshnetManager::sendImageToSlaves(image_message image)
{
    for (int i = 0; i < _slaves.size(); i++)
    {
        esp_now_send(_slaves[i].data(), (uint8_t *)&image, sizeof(image));
    }
}

void MeshnetManager::sendParametersToSlaves(ParameterID paramID, int value)
{
    parameter_message data;
    data.type = MESSAGE_TYPE_PARAMETER;
    data.paramID = paramID;
    data.value = value;

    for (int i = 0; i < _slaves.size(); i++)
    {
        if (esp_now_send(_slaves[i].data(), (uint8_t *)&data, sizeof(data)) != ESP_OK)
        {
            Serial.println("Error sending parameter data");
        }
        else
        {
            Serial.printf("Sent parameter data %d %d\n", data.paramID, data.value);
        }
    }
};

void MeshnetManager::sendSensorCommandToSlaves(sensor_message data)
{
    for (int i = 0; i < _slaves.size(); i++)
    {
        if (esp_now_send(_slaves[i].data(), (uint8_t *)&data, sizeof(data)) != ESP_OK)
        {
            Serial.println("Error sending sensor data");
        }
        else
        {
            // Serial.println("Sent sensor data" + data.name + " " + String(data.value));
        }
    }
}

#endif