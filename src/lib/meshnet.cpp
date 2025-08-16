#ifdef MESH_NET

#include <Arduino.h>
#include <WiFi.h>
#include <cstring>
#include <esp_now.h>
#include <string>
#include <vector>

#include "meshnet.h"
#include "shared.h"

void printMacAddress(const uint8_t *macAddress) {
  char macStr[18];
  sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1],
          macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
  Serial.println(macStr);
}

MeshnetManager *MeshnetManager::instance = nullptr;

MeshnetManager::MeshnetManager() {
  instance = this;
  init();
  Serial.println("MeshnetManager initialized");
}

void MeshnetManager::connectSlaves(std::vector<MacAddress> slaves) {
  _slaves = slaves;

  if (_slaves.empty()) {
    Serial.println("No slaves to connect to.");
    return;
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(MeshnetManager::OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));

  for (int i = 0; i < _slaves.size(); i++) {
    MacAddress mac = _slaves[i];
    memcpy(peerInfo.peer_addr, mac.data(), 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      printMacAddress(peerInfo.peer_addr);
    } else {
      Serial.println("Connected to Slave Board with MAC Address: ");
      printMacAddress(peerInfo.peer_addr);
    }
  }
}

void MeshnetManager::init() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(MeshnetManager::OnDataRecv);
}

std::string MeshnetManager::getMacAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  return std::string(reinterpret_cast<char *>(mac), 6);
}

void MeshnetManager::sendStringToSlaves(std::string command) {
  text_message msg;
  msg.numBytes = command.length();
  msg.type = MESSAGE_TYPE_TEXT;
  strcpy(msg.text, command.c_str());

  for (int i = 0; i < _slaves.size(); i++) {
    esp_now_send(_slaves[i].data(), reinterpret_cast<uint8_t *>(&msg),
                 sizeof(msg));
  }
}

void MeshnetManager::sendImageToSlaves(image_message image) {
  for (int i = 0; i < _slaves.size(); i++) {
    esp_now_send(_slaves[i].data(), reinterpret_cast<uint8_t *>(&image),
                 sizeof(image));
  }
}

void MeshnetManager::sendParametersToSlaves(ParameterID paramID, int value) {
  parameter_message data;
  data.type = MESSAGE_TYPE_PARAMETER;
  data.paramID = paramID;
  data.value = value;

  for (int i = 0; i < _slaves.size(); i++) {
    if (esp_now_send(_slaves[i].data(), reinterpret_cast<uint8_t *>(&data),
                     sizeof(data)) != ESP_OK) {
      // Serial.println("Error sending parameter data");
    } else {
      if (isVerbose())
        Serial.printf("Sent parameter data %d %d\n", data.paramID, data.value);
    }
  }
}

void MeshnetManager::sendSensorCommandToSlaves(sensor_message data) {
  for (int i = 0; i < _slaves.size(); i++) {
    if (esp_now_send(_slaves[i].data(), reinterpret_cast<uint8_t *>(&data),
                     sizeof(data)) != ESP_OK) {
      Serial.println("Error sending sensor data");
    } else {
      // Serial.println("Sent sensor data");
    }
  }
}

void MeshnetManager::handleImageMessage(image_message msg) {
  if (_imageHandler) {
    _imageHandler(msg);
  } else {
    Serial.println("No image handler");
  }
}

void MeshnetManager::handleTextMessage(text_message msg) {
  if (_textHandler) {
    _textHandler(msg);
  } else {
    Serial.println("No text handler");
  }
}

void MeshnetManager::handleSensorMessage(sensor_message msg) {
  if (_sensorHandler) {
    _sensorHandler(msg);
  } else {
    Serial.println("No sensor handler");
  }
}

void MeshnetManager::handleMessageSent(esp_now_send_status_t status) {
  (void)status;
  // optional: inspect status for debugging
}

void MeshnetManager::OnDataRecv(const uint8_t *mac_addr, const uint8_t *data,
                                int len) {
  (void)mac_addr;
  uint8_t messageType = *data;

  if (messageType == MESSAGE_TYPE_IMAGE) {
    image_message msg;
    if (len != sizeof(msg)) {
      Serial.printf("Unexpected data length received.%d vs %d\n", len,
                    (int)sizeof(msg));
      return;
    }
    memcpy(&msg, data, sizeof(msg));
    instance->handleImageMessage(msg);
  } else if (messageType == MESSAGE_TYPE_TEXT) {
    text_message msg;
    if (len != sizeof(msg)) {
      Serial.printf("Unexpected data length received.%d vs %d\n", len,
                    (int)sizeof(msg));
      return;
    }
    memcpy(&msg, data, sizeof(msg));
    instance->handleTextMessage(msg);
  } else if (messageType == MESSAGE_TYPE_SENSOR) {
    sensor_message msg;
    if (len != sizeof(msg)) {
      Serial.printf("Unexpected data length received.%d vs %d\n", len,
                    (int)sizeof(msg));
      return;
    }
    memcpy(&msg, data, sizeof(msg));
    instance->handleSensorMessage(msg);
  } else if (messageType == MESSAGE_TYPE_PARAMETER) {
    parameter_message msg;
    if (len != sizeof(msg)) {
      Serial.printf("Unexpected data length received.%d vs %d\n", len,
                    (int)sizeof(msg));
      return;
    }
    memcpy(&msg, data, sizeof(msg));
    if (instance->_parameterHandler) {
      instance->_parameterHandler(msg);
    } else {
      Serial.println("No parameter handler");
    }
  } else {
    Serial.printf("Unknown message type %d\n", messageType);
  }
}

void MeshnetManager::OnDataSent(const uint8_t *mac,
                                esp_now_send_status_t status) {
  (void)mac;
  instance->handleMessageSent(status);
}

#endif
