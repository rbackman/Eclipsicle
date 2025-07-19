#ifdef SLAVE_BOARD

#ifdef USE_LEDS

#include "lib/ledManager.h"

#endif

#include "lib/meshnet.h"

#include "lib/sensors.h"
#include "lib/serial.h"
#include "lib/shared.h"
#include "lib/config.h"
#include "lib/profiler.h"
#include "lib/string_utils.h"
#ifdef USE_MOTOR
#include "motors.h"
#endif
#include <vector>
#include <string>
#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

#ifdef DISPLAY_MANAGER
#include "lib/displayManager.h"
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

#ifdef DISPLAY_MANAGER
DisplayManager displayManager;
#endif
#ifdef USE_LEDS
LEDManager *ledManager;
#endif
#ifdef USE_SENSORS

SensorManager *sensorManager;
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
  if (msg.sensorId == SLIDER1)
  {
    parameter_message paramMsg;
    paramMsg.type = MESSAGE_TYPE_PARAMETER;
    paramMsg.paramID = PARAM_SPAWN_RATE;
    paramMsg.value = map(msg.value, 0, 255, 0, 100);
    ledManager->respondToParameterMessage(paramMsg);
  }
  if (msg.sensorId == BUTTON_UP)
  {
    ledManager->toggleMode();
  }

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

#ifdef USE_SENSORS
  SensorGrid grid = {
      SensorState(SLIDER, 34, SLIDER1),

      SensorState(BUTTON, 21, BUTTON_UP),
      SensorState(BUTTON, 25, BUTTON_DOWN),

  };
  sensorManager = new SensorManager(grid);
#endif
#ifdef TESSERATICA_SEGMENT
  makeRig("Tesseratica", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC});
  addStripToRig("Tesseratica", 0, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, 54.5, -54.5, -49.5}, {49, 54.5, -54.5, 50}, {73, 26.5, -26.5, 21}, {94, 26.5, -26.5, -21}, {122, 54.5, -54.5, -49.5}});
  addStripToRig("Tesseratica", 1, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, 49.5, -54.5f, -54.5}, {49, -49.5, -54.5, -54.5}, {73, -21, -26.5, -26.5}, {94, 21, -26.5, -26.5}, {122, 49.5, -54.5f, -54.5}});
  addStripToRig("Tesseratica", 2, 122, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, -54.5, -54.5, -49.5}, {49, -54.5, -54.5, 49.5}, {73, -26.5, -26.5, 21}, {94, -26.5, -26.5, -21}, {122, -54.5, -54.5, -49.5}});
#endif
#ifdef LIGHT_SWORD
  makeRig("Lightsword", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC});
  addStripToRig("Lightsword", 3, 128, LED_STATE_MULTI_ANIMATION,
                {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                {{0, 0.0f, -54.5f, -54.5f}, {127, 0.0f, 54.5f, 54.5f}});

#endif
  serialManager = new SerialManager(512, SLAVE_NAME);
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

#ifdef DISPLAY_MANAGER
  displayManager.begin();
  displayManager.showText("Hello, World!", 10, 10, 4, 0xFF);
#endif
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
  else if (command == "quiet")
  {
    setVerbose(false);
    Serial.println("Verbose mode: " + String(isVerbose() ? "ON" : "OFF"));
    return true;
  }
  else if (command.startsWith("p:"))
  {
    // command is in form "p:PARAM_ID:VALUE"
    int colonIndex = command.indexOf(':');
    if (colonIndex == -1)
    {
      Serial.println("Invalid command format. Expected 'p:PARAM_ID:VALUE'");
      return false;
    }
    auto parts = splitString(std::string(command.c_str()), ':');
    int paramID = toInt(parts[1]);
    std::string value = parts[2];
    ParameterID pid = (ParameterID)paramID;

    parameter_message parameter;
    parameter.paramID = pid;
    parameter.type = MESSAGE_TYPE_PARAMETER;

    if (isIntParameter(pid))
    {
      parameter.value = toInt(value);
    }
    else if (isFloatParameter(pid))
    {
      parameter.floatValue = toFloat(value);
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
  else if (command == "confirmParameters")
  {
    confirmParameters();

    return true;
  }
  else if (command == "saveDefaults")
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
  else if (command == "loadDefaults")
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
  else if (command == "resetDefaults")
  {
    configManager.clear();
    Serial.println("Defaults cleared");
    return true;
  }
  else if (command == "getStripState")
  {
    std::string state = ledManager->getStripsStateJson(true);
    Serial.println(String(state.c_str()) + ";");
    return true;
  }
  else if (command == "getStripStateCompact")
  {
    std::string state = ledManager->getStripStateCompact(true);
    String out = String(state.c_str());
    out.replace('\n', '|');
    Serial.println(out);
    return true;
  }
  else if (command == "confirmAnimations")
  {
    std::string info = ledManager->getAnimationInfoJson();
    Serial.println(String(info.c_str()) + ";");
    return true;
  }
#ifdef USE_LEDS
  else if (ledManager->handleLEDCommand(command))
  {
    // command was handled by LED manager
    return true;
  }
#endif

  else
  {
    if (isVerbose())
    {
      Serial.println("Command not recognized: " + command);
    }
  }

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
  {
    std::string state = ledManager->getStripState();
    display.print(String(state.c_str()));
  }

  display.display();
#endif

  return false; // command was not handled
}

void loop()
{
#ifdef USE_SENSORS
  sensorManager->updateSensors();
  if (sensorManager->messageAvailable())
  {
    sensor_message msg = sensorManager->getNextMessage();

    sensorHandler(msg);
  }
#endif
#ifdef ENABLE_PROFILER
  unsigned long loopStart = micros();
#endif
  serialManager->updateSerial();
  if (serialManager->stringAvailable())
  {
    String command = serialManager->readString();
#ifdef DISPLAY_MANAGER
    Serial.println("add to display: " + command);
    displayManager.showText(command, 0, 0);
#endif
    if (command.length() == 0)
      return;

    if (processCmd(command))
    {
      return; // command was handled
    }

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
#ifdef ENABLE_PROFILER
  unsigned long ledStart = micros();
#endif
  ledManager->update();
#ifdef ENABLE_PROFILER
  profilerAddLed(micros() - ledStart);
#endif
#endif

#ifdef ENABLE_PROFILER
  profilerAddLoop(micros() - loopStart);
  profilerMaybePrint();
#endif
  delay(5);
};
#endif