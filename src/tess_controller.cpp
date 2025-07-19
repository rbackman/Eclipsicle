#ifdef TESSERATICA_CONTROLLER
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#if DISPLAY_MANAGER
#include "./lib/displayManager.h"
#endif
#include "./lib/sensors.h"
#include "./lib/serial.h"
#define SDA_PIN 9
#define SCL_PIN 18
#define DATA_PIN 5
#define DOUT_PIN 8
#define MCP_CS 12
#define AMP_MIC_BLCK_PIN 37
#define AMP_MIC_LRC_PIN 38
#define AMP_DIN_PIN 42
#define MIC_SDA_PIN 39

#define LED_1_PIN 17
#define LED_2_PIN 16
#define LED_3_PIN 35
#define LED_4_PIN 48

#define SD_CARD_CS 10

#define DISPLAY_RST 15
#define DISPLAY_DC 14
#define DISPLAY_CS 13
#define DISPLAY_BL 4

#define BUTTON_1_PIN 1
#define BUTTON_2_PIN 2
#define BUTTON_3_PIN 41
#define BUTTON_4_PIN 40
#define BUTTON_5_PIN 19
#define BUTTON_6_PIN 47

#define ACCEL_SDA_PIN 20
#define ACCEL_SCL_PIN 21
#define ACCEL_INT_PIN 36

SerialManager *serialManager;
#if DISPLAY_MANAGER
DisplayManager *displayManager;
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