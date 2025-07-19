#pragma once
#ifdef SLAVE_BOARD
#include <Arduino.h>
#ifdef USE_LEDS
#include "lib/ledManager.h"
#endif

#ifdef DISPLAY_MANAGER
#include "lib/displayManager.h"
#endif

#ifdef USE_SENSORS

#include "lib/sensors.h"
#endif
#ifdef USE_MOTOR
#include "lib/motorManager.h"
#endif
#ifdef MESH_NET
#include "lib/meshnet.h"
#endif

#ifdef USE_PROFILER
#include "lib/profiler.h"
#endif
#include "lib/shared.h"
#include "lib/serial.h"

#include "lib/config.h"

#include "lib/string_utils.h"

#include <vector>
#include <string>

class SlaveBoard : public ParameterManager
{
private:
  SerialManager *serialManager;
  ConfigManager configManager;

#ifdef USE_LEDS
  LEDManager *ledManager;
#endif
#ifdef DISPLAY_MANAGER
  DisplayManager displayManager;
#endif

#ifdef USE_SENSORS

  SensorManager *sensorManager;
#endif
#ifdef USE_MOTOR
  MotorManager *motorManager;
#endif

#ifdef MESH_NET
  MeshnetManager *meshManager;
#endif

public:
  SlaveBoard(SerialManager *serialManager)
      : ParameterManager(SLAVE_NAME), serialManager(serialManager)
  {
    this->serialManager = serialManager;
// Constructor implementation
#ifdef MESH_NET
    meshManager = new MeshnetManager();
    meshManager->init();
#endif
#ifdef USE_LEDS
    ledManager = new LEDManager();

#endif

#ifdef DISPLAY_MANAGER
    displayManager = DisplayManager();
#endif

#ifdef USE_AUDIO
    audioManager = new AudioManager();

    audioManager->setVolume(50);          // Set volume to 50%
    audioManager->playTone(1000, 100, 5); // Play a test tone

#endif

#ifdef USE_SENSORS
    void setSensorManager(SensorManager * sensorManager)
    {
      this->sensorManager = sensorManager;
    }
#endif

    configManager.begin();
    configManager.loadParameters(this);
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
  }

  ~SlaveBoard() = default;

#ifdef USE_LEDS
  LEDManager *getLEDManager()
  {
    return ledManager;
  }
#endif

  void loop()
  {
    serialManager->updateSerial();
    if (serialManager->stringAvailable())
    {
      auto command = serialManager->readString();

      if (processCmd(command))
      {
        Serial.println("Command processed: " + command);
      }
      else
      {
        Serial.println("Command not recognized: " + command);
      }
    }
    // if (serialManager->jsonAvailable())
    // {
    //   JsonDocument doc;
    //   if (serialManager->readJson(doc))
    //   {
    //     processJson(doc);
    //   }
    // }
  }

  bool processCmd(String command)
  {

    if (command.startsWith("p:"))
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
    }
    else if (command == "confirmParameters")
    {
      confirmParameters();

      return true;
    }
    else if (command == "saveDefaults")
    {
      configManager.saveParameters(this);
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
      configManager.loadParameters(this);
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
    else if (ledManager->handleString(command))
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

  void processJson(JsonDocument &doc)
  {
    if (doc.isNull())
    {
      Serial.println("Received null JSON document");
      return;
    }

    if (doc.is<JsonObject>())
    {
      JsonObject obj = doc.as<JsonObject>();
      if (obj["type"].is<std::string>())
      {
        std::string type = obj["type"].as<std::string>();
        if (type == "parameter")
        {
          parameter_message parameter;
          parameter.paramID = (ParameterID)obj["id"].as<int>();
          parameter.type = MESSAGE_TYPE_PARAMETER;

          if (obj["intValue"].is<int>())
          {
            parameter.value = obj["intValue"].as<int>();
          }
          else if (obj["value"].is<int>())
          {
            parameter.value = obj["value"].as<int>();
          }
          else if (obj["floatValue"].is<float>())
          {
            parameter.floatValue = obj["floatValue"].as<float>();
          }
          else if (obj["boolValue"].is<bool>())
          {
            parameter.boolValue = obj["boolValue"].as<bool>();
          }
          else
          {
            Serial.println("Invalid parameter message");
            return;
          }
          //  TODO: do something with the parameter
        }
        else
        {
          Serial.println("Unknown JSON type: " + String(type.c_str()));
        }
      }
      else
      {
        Serial.println("JSON object does not contain 'type' key");
      }
    }
    else
    {
      Serial.println("Received non-object JSON document");
    }
  }
};

#endif