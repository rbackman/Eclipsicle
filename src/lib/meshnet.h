#pragma once

#include <functional>
#include <string>
#include <vector>

#include "shared.h"
#include <esp_now.h>

using ImageHandler = std::function<void(const image_message &)>;
using TextHandler = std::function<void(const text_message &)>;
using SensorHandler = std::function<void(const sensor_message &)>;
using ParameterHandler = std::function<void(const parameter_message &)>;

class MeshnetManager {
  static MeshnetManager *instance; // For access in the callback

public:
  MeshnetManager();

  void connectSlaves(std::vector<MacAddress> slaves);
  void sendSensorCommandToSlaves(sensor_message msg);
  void sendImageToSlaves(image_message image);
  void sendParametersToSlaves(ParameterID paramID, int value);
  void sendStringToSlaves(std::string msg);
  void init();

  void setImageHandler(ImageHandler handler) { _imageHandler = handler; }
  void setTextHandler(TextHandler handler) { _textHandler = handler; }
  void setSensorHandler(SensorHandler handler) { _sensorHandler = handler; }
  void setParameterHandler(ParameterHandler handler) {
    _parameterHandler = handler;
  }
  std::string getMacAddress();

private:
  ImageHandler _imageHandler = nullptr;
  TextHandler _textHandler = nullptr;
  SensorHandler _sensorHandler = nullptr;
  ParameterHandler _parameterHandler = nullptr;

  std::vector<MacAddress> _slaves;

  void handleImageMessage(image_message msg);
  void handleTextMessage(text_message msg);
  void handleSensorMessage(sensor_message msg);
  void handleMessageSent(esp_now_send_status_t status);

  static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len);
  static void OnDataSent(const uint8_t *mac, esp_now_send_status_t status);
};
