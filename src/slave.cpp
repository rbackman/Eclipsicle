#ifdef TESS_BOARD
#include "slave.h"
#include "boardConfigs/ledNodes.h"
#include "boardConfigs/ledNodes.h"

SlaveBoard::SlaveBoard(SerialManager *serialManager)
    : ParameterManager(SLAVE_NAME, {PARAM_SHOW_FPS}),
      serialManager(serialManager)
{
    this->serialManager = serialManager;
    // Constructor implementation

#ifdef TESSERATICA_SEGMENT

    const auto &structure = generateFullTesseratica();
    // serializeStructure(structure);
    // Serial.println("Structure: ");
    // serializeJson(structure, Serial);
    // Serial.println();

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
#ifdef MESH_NET
    meshManager = new MeshnetManager();
#ifdef USE_LEDS
    meshManager->setImageHandler([this](const image_message &msg)
                                 { ledManager->setLEDImage(msg); });
#endif
#ifdef LED_MASTER
    meshManager->setTextHandler([this](const text_message &msg)
                                { uartManager.broadcastString(msg.text); });
#else
    meshManager->setTextHandler([this](const text_message &msg)
                                { handleTextMessage(msg.text); });
#endif
    // meshManager->setSensorHandler([this](const sensor_message &msg)
    //                               { handleSensorMessage(msg); });
    meshManager->setParameterHandler([this](const parameter_message &msg)
                                     { handleParameterMessage(msg); });
    meshManager->init();

#endif
#ifdef LED_MASTER
    uartManager.beginMaster();
    uartManager.addSlave(0x10, 46, 47, 2);
    uartManager.addSlave(0x11, 8, 9, 1);
    uartManager.addSlave(0x12, 37, 38, 0);
    uartManager.testSlaves();
#endif
#ifdef LED_SLAVE
#ifndef UART_ADDRESS
#define UART_ADDRESS 0x10
#endif
    uartManager.beginSlave(UART_ADDRESS, [this](const std::string &msg)
                           { handleString(String(msg.c_str())); });
#endif
#ifdef USE_SENSORS
    void setSensorManager(SensorManager * sensorManager)
    {
        this->sensorManager = sensorManager;
    }
#endif

    configManager.begin();
    configManager.loadParameters(this);
#ifdef USE_LEDS
    configManager.loadParameters(ledManager);
    for (auto strip : ledManager->getStrips())
    {
        configManager.loadParameters(strip);
        for (auto &anim : strip->getAnimations())
        {
            configManager.loadParameters(anim.get());
        }
    }
#endif
    if (isVerbose())
    {
        sanityCheckParameters();
    }
}

bool SlaveBoard::handleParameterMessage(parameter_message parameter)
{

    if (isVerbose())
    {
        printParameterMessage(parameter);
    }

#ifdef USE_LEDS
    if (ledManager && ledManager->handleParameterMessage(parameter))
    {
        return true;
    }
#endif
#ifdef USE_SENSORS
    if (sensorManager && sensorManager->handleParameterMessage(parameter))
    {
        return true;
    }
#endif
#ifdef USE_MOTOR
    if (motorManager && motorManager->handleParameterMessage(parameter))
    {
        return true;
    }
#endif
    return ParameterManager::handleParameterMessage(parameter);
}

void SlaveBoard::loop()
{
    serialManager->updateSerial();
    uartManager.update();
    if (serialManager->stringAvailable())
    {
        auto command = serialManager->readString();

        if (handleString(command))
        {
            // Serial.println("Command processed: " + command);
        }
        else
        {
            if (isVerbose())
                Serial.println("Command not recognized: " + command);
        }
    }
    if (serialManager->jsonAvailable())
    {
        JsonDocument doc;
        if (serialManager->readJson(doc))
        {
            handleJson(doc);
        }
    }
#ifdef USE_LEDS
    ledManager->update();
#endif
#ifdef USE_SENSORS
    if (sensorManager)
    {
        sensorManager->update();
    }
#endif
#ifdef USE_MOTOR
    if (motorManager)
    {
        motorManager->update();
    }
#endif

#ifdef USE_PROFILER
    Profiler::getInstance().update();
#endif

#ifdef LED_MASTER
    static uint32_t lastSync = 0;
    uint32_t now = millis();
    if (now - lastSync > 1000)
    {
        uartManager.sendSync(now);
        lastSync = now;
    }
#endif
}

bool SlaveBoard::handleString(String command)
{

    if (command.startsWith("p:"))
    {
#ifdef LED_MASTER
        uartManager.broadcastString(command.c_str());
#endif
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
        return handleParameterMessage(parameter);
    }
    else if (command == "confirmParameters")
    {
        confirmParameters();

        return true;
    }
#ifdef USE_LEDS
    else if (command.startsWith("menu:"))
    {
        // command is in form "menu:MENU_NAME"
        std::string menuName = command.substring(5).c_str();

        ledManager->setAnimation(menuName);
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
#endif
    else if (command == "resetDefaults")
    {
        configManager.clear();
        Serial.println("Defaults cleared");
        return true;
    }
    else
    {

#ifdef LED_MASTER
        uartManager.broadcastString(command.c_str());
#endif
#ifdef USE_LEDS
        if (ledManager->handleString(command))
        {
            return true;
        }
#endif

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
    return true; // command was handled
}

void SlaveBoard::handleJson(JsonDocument &doc)
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

#endif
