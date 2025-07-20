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
  SlaveBoard(SerialManager *serialManager);

#ifdef USE_LEDS
  LEDManager *getLEDManager()
  {
    return ledManager;
  }
#endif

  void loop();
  bool handleString(String command);
  void handleJson(JsonDocument &doc);
  bool handleParameterMessage(parameter_message parameter);
};

#endif