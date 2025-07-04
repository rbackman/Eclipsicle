#ifdef SLAVE_BOARD

#ifdef USE_LEDS

#include "ledManager.h"

#endif

#include "meshnet.h"

#include "sensors.h"
#include "serial.h"
#include "shared.h"
#include "config.h"
#ifdef USE_MOTOR
#include "motors.h"
#endif
#include <vector>
#include <string>
#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

#ifdef USE_ACCELEROMETER
#include <Adafruit_MPU6050.h>
#endif
#ifdef USE_SENSORS
#include <Adafruit_Sensor.h>
#endif
#include <Wire.h>

#ifdef USE_ACCELEROMETER
Adafruit_MPU6050 mpu;
#endif

int lastButtonState = HIGH; // Assume button starts unpressed
bool showAccel = false;

#ifdef USE_DISPLAY
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
#endif
JsonDocument doc;
SerialManager *serialManager;
ParameterManager *parameterManager;

#ifdef USE_LEDS
LEDManager *ledManager;
#endif

ConfigManager configManager;

#ifdef MESH_NET
MeshnetManager *meshManager;
#endif
#ifdef USE_MOTOR
MotorManager *motorManager;
#endif

#ifdef USE_LEDS
ImageHandler imageHandler = [](image_message msg)
{
  if (isVerbose())
    Serial.println("image received:  " + String(msg.numBytes));
  ledManager->setLEDImage(msg);
};

TextHandler textHandler = [](text_message msg)
{
  Serial.println("Text received: " + String(msg.text));

  ledManager->handleLEDCommand(msg.text);
};
#endif

#ifdef USE_SENSORS
SensorHandler sensorHandler = [](sensor_message msg)
{
#ifdef USE_MOTOR
  if (msg.name == "Slider5")
  {
    motorManager->setMotor(msg.value * 180 / 255);
  }
  else
  {
    if (isVerbose())
    {
      Serial.println("Sensor received: " + String(msg.name) + " " + String(msg.value));
    }
    ledManager->respondToSensor(msg);
  }
#else
  if (isVerbose())
  {
    Serial.println("Sensor received: " + String(getSensorName(msg.sensorId)) + " " + String(msg.value));
  }
#endif
#ifdef USE_LEDS
#endif
};

#endif

ParameterHandler parameterHandler = [](parameter_message msg)
{
  if (msg.paramID == PARAM_DISPLAY_ACCEL)
  {
    showAccel = msg.boolValue;
  }

  parameterManager->respondToParameterMessage(msg);

#ifdef USE_LEDS
  ledManager->respondToParameterMessage(msg);
#endif
#ifdef USE_MOTOR
  motorManager->respondToParameterMessage(msg);
#endif
};

bool respondToParameterChange(parameter_message parameter)
{

  return ledManager->respondToParameterMessage(parameter);
}

void setup()
{

  serialManager = new SerialManager(220, SLAVE_NAME);
  configManager.begin();
  parameterManager = new ParameterManager(SLAVE_NAME, {PARAM_DISPLAY_ACCEL});
  ledManager = new LEDManager(SLAVE_NAME);
  configManager.loadParameters(parameterManager);
  configManager.loadParameters(ledManager);
  for (auto strip : ledManager->getStrips())
  {
    configManager.loadParameters(strip);
    for (auto &anim : strip->getAnimations())
    {
      configManager.loadParameters(anim.get());
    }
  }
  if (isVerbose())
  {
    sanityCheckParameters();
  }

  parameterManager->addParameterChangeListener(respondToParameterChange);
#ifdef USE_DISPLAY
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);

  Serial.println("Adafruit MPU6050 found display!");
#endif

#ifdef USE_ACCELEROMETER
  // setup accelerometer
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
  }

  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("MPU6050 Found!");

  // setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
#endif

#ifdef USE_MOTOR
  motorManager = new MotorManager();
#endif
#ifdef MESH_NET
  meshManager = new MeshnetManager();

  meshManager->setImageHandler(imageHandler);
  meshManager->setTextHandler(textHandler);
  meshManager->setParameterHandler(parameterHandler);
#ifdef USE_SENSORS
  meshManager->setSensorHandler(sensorHandler);
#endif
#endif

  Serial.println("Free memory Setup: ");
  Serial.println(ESP.getFreeHeap());
}
static void confirmParameters()
{
  Serial.println("Confirming parameters...");

  JsonDocument paramJson; // adjust size as needed
  paramJson["type"] = "parameters";
  JsonObject data = paramJson["data"].to<JsonObject>();

  auto paramList = getParameterNames();

  for (const auto &bParam : getDefaultBoolParameters())
  {
    const char *name = paramList[bParam.id].c_str();
    JsonObject obj = data[name].to<JsonObject>();
    obj["id"] = bParam.id;
    obj["type"] = "bool";
    obj["value"] = bParam.value;
    obj["name"] = bParam.name;
  }

  for (const auto &iParam : getDefaultIntParameters())
  {
    const char *name = paramList[iParam.id].c_str();
    JsonObject obj = data[name].to<JsonObject>();
    obj["id"] = iParam.id;
    obj["type"] = "int";
    obj["value"] = iParam.value;
    obj["name"] = iParam.name;
    obj["min"] = iParam.min;
    obj["max"] = iParam.max;
  }

  for (const auto &fParam : getDefaultFloatParameters())
  {
    const char *name = paramList[fParam.id].c_str();
    JsonObject obj = data[name].to<JsonObject>();
    obj["id"] = fParam.id;
    obj["type"] = "float";
    obj["value"] = fParam.value;
    obj["name"] = fParam.name;
    obj["min"] = fParam.min;
    obj["max"] = fParam.max;
  }

  serializeJson(paramJson, Serial);
  Serial.println(";");
}

bool processCmd(String command)
{

  if (command == "verbose")
  {
    setVerbose(true);
    Serial.println("Verbose mode: " + String(isVerbose() ? "ON" : "OFF"));
    return true;
  }
  if (command == "quiet")
  {
    setVerbose(false);
    Serial.println("Verbose mode: " + String(isVerbose() ? "ON" : "OFF"));
    return true;
  }
  if (command.startsWith("p:"))
  {
    // command is in form "p:PARAM_ID:VALUE"
    int colonIndex = command.indexOf(':');
    if (colonIndex == -1)
    {
      Serial.println("Invalid command format. Expected 'p:PARAM_ID:VALUE'");
      return false;
    }
    auto parts = splitString(command, ':');
    int paramID = parts[1].toInt();
    String value = parts[2];
    ParameterID pid = (ParameterID)paramID;

    parameter_message parameter;
    parameter.paramID = pid;
    parameter.type = MESSAGE_TYPE_PARAMETER;

    if (isIntParameter(pid))
    {
      parameter.value = value.toInt();
    }
    else if (isFloatParameter(pid))
    {
      parameter.floatValue = value.toFloat();
    }
    else if (isBoolParameter(pid))
    {
      parameter.boolValue = (value == "true" || value == "1");
    }
    else
    {
      Serial.println("Unknown parameter ID: " + String(paramID));
      return false;
    }
    parameterHandler(parameter);
  }
  if (command == "confirmParameters")
  {
    confirmParameters();

    return true;
  }
  if (command == "saveDefaults")
  {
    configManager.saveParameters(parameterManager);
    configManager.saveParameters(ledManager);
    for (auto strip : ledManager->getStrips())
    {
      configManager.saveParameters(strip);
      for (auto &anim : strip->getAnimations())
      {
        configManager.saveParameters(anim.get());
      }
    }
    Serial.println("Defaults saved");
    return true;
  }
  if (command == "loadDefaults")
  {
    configManager.loadParameters(parameterManager);
    configManager.loadParameters(ledManager);
    for (auto strip : ledManager->getStrips())
    {
      configManager.loadParameters(strip);
      for (auto &anim : strip->getAnimations())
      {
        configManager.loadParameters(anim.get());
      }
    }
    Serial.println("Defaults loaded");
    return true;
  }
  if (command == "resetDefaults")
  {
    configManager.clear();
    Serial.println("Defaults cleared");
    return true;
  }
  if (command == "getStripState")
  {
    String state = ledManager->getStripStateJson(true);
    Serial.println(state + ";");
    return true;
  }

  // float ax = a.acceleration.x;
  // float ay = a.acceleration.y;
  // float az = a.acceleration.z;
  // float magnitude = sqrt(ax * ax + ay * ay + az * az);
  // ax /= magnitude;
  // ay /= magnitude;
  // az /= magnitude;

  // // Calculate the angle in the x-y plane
  // float angle = atan2(ay, ax); // Radians

  // // Map the angle to an LED index
  // float gravityPosition = ((angle + PI) / (2 * PI));

  // ledManager->setGravityPosition(gravityPosition);

#ifdef USE_DISPLAY
  display.clearDisplay();
  display.setCursor(0, 0);
  if (showAccel)
  {

    display.print("Accel ");
    display.print(a.acceleration.x, 1);
    display.print(",");
    display.print(a.acceleration.y, 1);
    display.print(",");
    display.print(a.acceleration.z, 1);
    display.println("");

    display.print("Gyro");
    display.print(g.gyro.x, 1);
    display.print(",");
    display.print(g.gyro.y, 1);
    display.print(",");
    display.print(g.gyro.z, 1);
    display.println("");
  }
  display.print(ledManager->getStripState());

  display.display();
#endif

  return false; // command was not handled
}

void loop()
{
  serialManager->updateSerial();
  if (serialManager->stringAvailable())
  {
    String command = serialManager->readString();
    if (command.length() == 0)
      return;

    if (processCmd(command))
    {
      return; // command was handled
    }
#ifdef USE_LEDS

    ledManager->handleLEDCommand(command);
#endif

#ifdef USE_MOTOR
    if (motorManager->handleMotorCommand(command))
    {
      return; // command was handled by motor manager
    }

#endif
  }
  else if (serialManager->jsonAvailable())
  {
    bool handled = false;
    if (serialManager->readJson(doc))
    {

      if (parameterManager->handleJsonMessage(doc))
      {
        handled = true;
      }
    }
    else
    {
      Serial.println("Json not handled");
      serializeJson(doc, Serial);
      Serial.println();
    }
  }
#ifdef USE_LEDS
  ledManager->update();
#endif

  delay(20);
};
#endif