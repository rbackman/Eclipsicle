#ifdef TESSERATICA_CONTROLLER
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

#include "./boardConfigs/tessControllerPins.h"

SerialManager *serialManager;
#if DISPLAY_MANAGER
DisplayManager *displayManager;
#endif
MeshnetManager *meshManager;
#ifdef USE_AUDIO
AudioManager *audioManager;
#endif

SensorManager *sensorManager;
SPIClass sensorSPI(HSPI);
int sliderValues[5] = {0};
// SPI settings: 1 MHz, MSB first, SPI mode 0
// SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

void setup()
{
  serialManager = new SerialManager(1024, SLAVE_NAME);
  delay(300);
  sensorSPI.begin(SCL_PIN, DOUT_PIN, SDA_PIN, MCP_CS);
  pinMode(MCP_CS, OUTPUT);
  digitalWrite(MCP_CS, HIGH); // Set MCP CS high to deselect it

  sensorManager = new SensorManager({
                                        SensorState(BUTTON, BUTTON_2_PIN, BUTTON_UP),
                                        SensorState(BUTTON, BUTTON_6_PIN, BUTTON_DOWN),
                                        SensorState(BUTTON, BUTTON_5_PIN, BUTTON_LEFT),
                                        SensorState(BUTTON, BUTTON_4_PIN, BUTTON_RIGHT),
                                        SensorState(SLIDER, 3, SLIDER1, MCP_CS),
                                        SensorState(SLIDER, 2, SLIDER2, MCP_CS),
                                        SensorState(SLIDER, 1, SLIDER3, MCP_CS),
                                        SensorState(SLIDER, 4, SLIDER4, MCP_CS),
                                        SensorState(SLIDER, 0, SLIDER5, MCP_CS),
                                    },
                                    &sensorSPI);

#ifdef USE_AUDIO
  audioManager = new AudioManager(AMP_MIC_BLCK_PIN, AMP_MIC_LRC_PIN, AMP_DIN_PIN, MIC_SDA_PIN);
  audioManager->begin();
  audioManager->setVolume(50);          // Set volume to 50%
  audioManager->playTone(1000, 100, 5); // Play a test tone
  audioManager->setMicGain(50);         // Set microphone gain to 50%
#endif

  meshManager = new MeshnetManager();
  meshManager->init();
  meshManager->connectSlaves();

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