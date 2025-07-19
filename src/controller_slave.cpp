#ifdef CONTROLLER_SLAVE
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#if DISPLAY_MANAGER
#include "./lib/displayManager.h"
#endif
#include "./lib/sensors.h"
#include "./lib/serial.h"
#include "./lib/meshnet.h"
#include "./lib/audio.h"
#include "./lib/shared.h"
#include "./lib/ledManager.h"
#include "./boardConfigs/tessControllerPins.h"

SerialManager *serialManager;
#if DISPLAY_MANAGER
DisplayManager *displayManager;
#endif
MeshnetManager *meshManager;
#ifdef USE_AUDIO
AudioManager *audioManager;
#endif

LEDManager *ledManager;

SensorManager *sensorManager;
SPIClass sensorSPI(HSPI);
int sliderValues[5] = {0};
// SPI settings: 1 MHz, MSB first, SPI mode 0
// SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

void on_sensor_message(sensor_message msg)
{
  if (msg.sensorType == SLIDER)
  {
    int index = msg.sensorId - SLIDER1;
    if (index >= 0 && index < 5)
    {
      sliderValues[index] = msg.value;
    }
  }
}
void on_text_message(text_message msg)
{
  if (isVerbose())
  {
    Serial.println("Text message received: " + String(msg.text));
  }
}
void on_image_message(image_message msg)
{
  if (isVerbose())
  {
    Serial.println("Image message received: " + String(msg.numBytes));
  }
  ledManager->setLEDImage(msg);
}
void on_parameter_message(const parameter_message &msg)
{
  if (isVerbose())
  {
    Serial.println("Parameter message received: " + String(msg.paramID) + " = " + String(msg.value));
  }
  parameterManager->setParameter(msg.paramID, msg.value);
}
void setup()
{
  makeRig("ControllerSlave", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC});
  makeRig("ControllerSlave", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC});
  addStripToRig("ControllerSlave", 0, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, 54.5, -54.5, -49.5}, {49, 54.5, -54.5, 50}, {73, 26.5, -26.5, 21}, {94, 26.5, -26.5, -21}, {122, 54.5, -54.5, -49.5}});
  addStripToRig("ControllerSlave", 1, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, 49.5, -54.5f, -54.5}, {49, -49.5, -54.5, -54.5}, {73, -21, -26.5, -26.5}, {94, 21, -26.5, -26.5}, {122, 49.5, -54.5f, -54.5}});
  addStripToRig("ControllerSlave", 2, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, -54.5, -54.5, -49.5}, {49, -54.5, -54.5, 49.5}, {73, -26.5, -26.5, 21}, {94, -26.5, -26.5, -21}, {122, -54.5, -54.5, -49.5}});

  serialManager = new SerialManager(1024, SLAVE_NAME);

  meshManager = new MeshnetManager();
  meshManager->init();
  meshManager->setTextHandler(on_text_message);
  meshManager->setImageHandler(on_image_message);
  meshManager->setSensorHandler(on_sensor_message);
  meshManager->setParameterHandler(on_parameter_message);

#if DISPLAY_MANAGER
  displayManager = new DisplayManager();
  displayManager->begin(DISPLAY_DC, DISPLAY_CS, SCL_PIN, SDA_PIN,
                        DISPLAY_RST, DISPLAY_BL, &sensorSPI, DOUT_PIN);
  //  turn on the display backlight

  displayManager->showText("Tesseratica Controller", 10, 10, 2, 0xFF);
  displayManager->showText("Ready!", 10, 30, 2, 0xFF);
  // show macaddress
  String macAddress = WiFi.macAddress();
  displayManager->showText("MAC: " + macAddress, 10, 50, 2, 0xFF);
#endif

  delay(1000); // Wait for serial monitor to open
}

void loop()
{
  serialManager->updateSerial();
  // displayManager->clear();
  sensorManager->updateSensors();

  if (sensorManager->messageAvailable())
  {
    sensor_message message = sensorManager->getNextMessage();
    String sensorLabel = getSensorName(message.sensorId);
    String sensorValue = String(message.value);
    if (isVerbose())
    {
      String txt = "Sensor: " + sensorLabel + " Value: " + sensorValue;
      Serial.println(txt.c_str());
    }
    if (message.sensorType == SLIDER)
    {
      int index = message.sensorId - SLIDER1;
      if (index >= 0 && index < 5)
      {
        sliderValues[index] = message.value;
#if DISPLAY_MANAGER

        // displayManager->showBars(sliderValues, 5, 90, 120, 360, 60);
        // convert value to hue

        float barHeight = (float)message.value / 1023.0f; // Scale to bar height
        displayManager->drawBar(index, 100, 300, 60, barHeight, 200, barHeight);
#endif
      }
    }
  }
  if (serialManager->stringAvailable())
  {
    String command = serialManager->readString();
    if (sensorManager->handleSensorCommand(command))
    {
      Serial.println("Handled command: " + command);
    }
  }
  if (serialManager->jsonAvailable())
  {
    JsonDocument doc;
    if (serialManager->readJson(doc))
    {
      // Handle JSON commands here
      String command = doc["command"];
    }
  }
}
#endif