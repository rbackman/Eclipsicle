
#pragma once
#ifdef MASTER_BOARD

#include <Arduino.h>
#ifdef USE_LEDS
#include "lib/ledManager.h"
#endif

#ifdef DISPLAY_MANAGER
#include "lib/displayManager.h"
#endif

#ifdef USE_AUDIO
#include "lib/audio.h"
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
#include "lib/parameterManager.h"
#include "lib/menuManager.h"
#include "lib/config.h"
#include "lib/slideshow.h"

#include "lib/string_utils.h"

#include <vector>
#include <string>

class MasterBoard : public ParameterManager
{
private:
    SerialManager *serialManager = nullptr;
    ConfigManager configManager;
    MenuManager *menuManager = nullptr;
    SPIClass spiBus = SPIClass(HSPI);
    void init();
#ifdef USE_LEDS
    LEDManager *ledManager = nullptr;
#endif
#ifdef DISPLAY_MANAGER
    DisplayManager *displayManager = nullptr;
#endif
#ifdef USE_AUDIO
    AudioManager *audioManager = nullptr;
#endif

#ifdef USE_SENSORS

    SensorManager *sensorManager = nullptr;
#endif
#ifdef USE_MOTOR
    MotorManager *motorManager = nullptr;
#endif

#ifdef MESH_NET
    MeshnetManager *meshManager = nullptr;
#endif

    Slideshow *slideshow = nullptr;

public:
    MasterBoard()
        : ParameterManager(SLAVE_NAME, allParameterIDs)
    {
        init();
    }

#ifdef USE_LEDS
    LEDManager *getLEDManager()
    {
        return ledManager;
    }
#endif
    bool processSensorMessage(sensor_message message);
    void update();
    bool handleTextMessage(std::string command);
    void handleJson(JsonDocument &doc);
    bool handleParameterMessage(parameter_message parameter);
    void menuChangedParameters(parameter_message listener);
    void renderParameterMenu(bool forceClear = false);
};

#endif
