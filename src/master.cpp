#ifdef MASTER_BOARD

#include <Arduino.h>

#include <SPI.h>

#include <ArduinoJson.h>
#include "master.h"
#include "lib/pins.h"

void MasterBoard::init()
{

    serialManager = new SerialManager(512);
    menuManager = new MenuManager();
    menuManager->selectMenu(MENU_MAIN);
    meshManager = new MeshnetManager();

    std::vector<MacAddress> slaves = {
   
        {0xD8, 0x3B, 0xDA, 0x77, 0xD9, 0x34}, // Slave0 D8:3B:DA:77:D9:34
        // {0x98, 0x88, 0xE0, 0x03, 0xFF, 0x58}, // Slave1 98:88:E0:03:FF:58
//98:88:E0:04:23:74
        {0x98, 0x88, 0xE0, 0x04, 0x23, 0x74}, // Slave1 98:88:E0:04:23:74
    };
    meshManager->connectSlaves(slaves);

    spiBus.begin(SCL_PIN, DOUT_PIN, SDA_PIN, MCP_CS);
    pinMode(MCP_CS, OUTPUT);
    digitalWrite(MCP_CS, HIGH); // Set MCP CS high to deselect it

    sensorManager = new SensorManager({
                                          SensorState(BUTTON, BUTTON_2_PIN, BUTTON_UP),
                                          SensorState(BUTTON, BUTTON_6_PIN, BUTTON_DOWN),
                                          SensorState(BUTTON, BUTTON_5_PIN, BUTTON_LEFT),
                                          SensorState(BUTTON, BUTTON_4_PIN, BUTTON_RIGHT),
                                          SensorState(SLIDER, 3, SLIDER1, MCP_CS, 2),
                                          SensorState(SLIDER, 2, SLIDER2, MCP_CS, 2),
                                          SensorState(SLIDER, 1, SLIDER3, MCP_CS, 2),
                                          SensorState(SLIDER, 4, SLIDER4, MCP_CS, 2),
                                          SensorState(SLIDER, 0, SLIDER5, MCP_CS, 2),
                                      },
                                      &spiBus);

#ifdef DISPLAY_MANAGER
    displayManager = new DisplayManager();
    displayManager->begin(&spiBus);
    // displayManager = new DisplayManager();
    //   displayManager->begin(DISPLAY_DC, DISPLAY_CS, SCL_PIN, SDA_PIN,
    //                         DISPLAY_RST, DISPLAY_BL, &sensorSPI, DOUT_PIN);
    //   //  turn on the display backlight

    displayManager->showText("Tesseratica Controller", 10, 10, 2, 0xFF);
    displayManager->showText("Ready!", 10, 30, 2, 0xFF);
    //   // show macaddress
    std::string macAddress = meshManager->getMacAddress();
    std::string text = "MAC: " + macAddress;
    displayManager->showText(text.c_str(), 10, 50, 2, 0xFF);
    delay(2000); // Wait for 2 seconds to show the text
#endif

#ifdef USE_AUDIO
    audioManager = new AudioManager();
    // audioManager->playTone(1500, 100, 5);
#endif

    Serial.println("\n\nMaster initialized\n\n");
};

bool MasterBoard::processSensorMessage(sensor_message message)
{
    bool used = false;
    if (message.type != MESSAGE_TYPE_SENSOR)
    {
        Serial.println("Eror Not a sensor message");
        return false;
    }

#ifdef USE_AUDIO

    // if (message.sensorId == BUTTON_TRIGGER)
    // {

    //     if (currentMenu == MENU_AUDIO)
    //     {
    //         if (!audioManager->isRecordingAudio())
    //         {
    //             Serial.println("Recording Audio");
    //             audioManager->record();
    //         }
    //         else
    //         {
    //             Serial.println("Playing Audio");
    //             audioManager->stop();
    //         }
    //     }
    // }
    // if (message.sensorId == BUTTON_RIGHT)
    // {
    //     audioManager->play();
    // }
    // else if (message.sensorId == BUTTON_UP)
    // {
    //     audioManager->debugAudio();
    // }

    if (message.sensorId == BUTTON_TRIGGER)
    {
        audioManager->playTone(1000, 100, 5);
        meshManager->sendParametersToSlaves(PARAM_BEAT, 255);
        Serial.println("Trigger Button");
    }
    if (message.sensorId == BUTTON_LEFT)
    {
        audioManager->playTone(800, 100, 5);
    }
    if (message.sensorId == BUTTON_RIGHT)
    {
        audioManager->playTone(600, 100, 5);
    }
    if (message.sensorId == BUTTON_UP)
    {
        audioManager->playTone(400, 100, 5);
    }
    if (message.sensorId == BUTTON_DOWN)
    {
        audioManager->playTone(200, 100, 5);
    }

#endif

    return menuManager->handleSensorMessage(message);
};
bool MasterBoard::handleTextMessage(std::string command)
{
    if (menuManager->handleTextMessage(command))
    {
        return true;
    }
    return ParameterManager::handleTextMessage(command);
}
void MasterBoard::update()
{
#ifdef USE_AUDIO
    // audioManager->update();

    // auto decibels = audioManager->getDecibel();
    // if (decibels > 0)
    // {
    //     Serial.println("Decibels: " + String(decibels));
    // }
#endif
    sensorManager->updateSensors();
    serialManager->updateSerial();

    if (sensorManager->messageAvailable())
    {

        sensor_message message = sensorManager->getNextMessage();

        if (!processSensorMessage(message))
        {

            // Serial.println("Send Sensor Message to slaves: " + message.name + " " + String(message.value));
            // meshManager->sendSensorCommandToSlaves(message);
        }
    }

    if (serialManager->stringAvailable())
    {
        String message = serialManager->readString();
        //   Serial.print("Send Serial Message: ");

        // Serial.print("Send Serial Message: " + message);
        if (handleTextMessage(message.c_str()) == false)
            meshManager->sendStringToSlaves(message);
    }
    if (serialManager->jsonAvailable())
    {
        JsonDocument doc;

        if (
            serialManager->readJson(doc))
        {
            handleJson(doc);
            serialManager->clearBuffer();
        }
    }
    if (menuManager->menuChanged())
    {

        auto menuPath = menuManager->getMenuName(menuManager->getCurrentMenu(), 16);

        auto stringCmd = "menu:" + menuPath;
        if (isVerbose())
        {
            Serial.printf(stringCmd.c_str());
        }
        meshManager->sendStringToSlaves(stringCmd.c_str());
        if (menuManager->getMenuMode() == MENU_MODE_EDIT_MODE)
        {
            renderParameterMenu(true);
        }
        else
        {
            auto menuItems = menuManager->getMenuItems();
            displayManager->displayMenu(menuItems, 0);
        }
    }
    if (menuManager->selectionChanged())
    {
        int selected = menuManager->getSelectedIndex();
        if (menuManager->getMenuMode() == MENU_MODE_EDIT_MODE)
        {
            renderParameterMenu();
        }
        else
        {
            auto menuItems = menuManager->getMenuItems();
            displayManager->displayMenu(menuItems, selected);
        }
    }
    if (menuManager->parameterChanged())
    {
        auto message = menuManager->getMessage();
        if (isIntParameter(message.paramID))
        {
            auto p = getIntParameter(message.paramID);
            message.value = lerp(0, 1023, p.min, p.max, message.value);
            setInt(message.paramID, message.value);
        }
        else if (isFloatParameter(message.paramID))
        {
            auto p = getFloatParameter(message.paramID);
            float val = ((float)message.value / 1023.0f) * (p.max - p.min) + p.min;
            setFloat(message.paramID, val);
            message.value = (int)val;
        }
        else if (isBoolParameter(message.paramID))
        {
            bool val = message.value < 512 ? false : true;
            setBool(message.paramID, val);
            message.value = val ? 1 : 0;
        }
        else
        {
            Serial.printf("Error: Parameter %d not found\n", message.paramID);
            return;
        }
        if (isVerbose())
            Serial.println("sending to slaves  : " + String(message.paramID) + " " + String(message.value));
        meshManager->sendParametersToSlaves(message.paramID, message.value);
        if (menuManager->getMenuMode() == MENU_MODE_EDIT_MODE)
        {
            renderParameterMenu();
        }
    }
}

void MasterBoard::handleJson(JsonDocument &doc)
{
    if (doc["pixleByte"].is<String>())
    {

        image_message msg;
        msg.type = MESSAGE_TYPE_IMAGE;
        msg.width = doc["width"];
        msg.row = doc["row"];
        msg.size = doc["size"];
        msg.numBytes = doc["numBytes"];
        String pixelByteString = doc["pixleByte"].as<String>();
        pixelByteString.trim();

        // Serial.println("\npixelByteString: " + pixelByteString);
        memcpy(msg.pixleBytes, (char *)pixelByteString.c_str(), msg.numBytes + 1);

        meshManager->sendImageToSlaves(msg);
    }
    else
    {
    }
}

bool MasterBoard::handleParameterMessage(parameter_message parameter)
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

    ParameterManager::handleParameterMessage(parameter);

    // Handle the case where no manager processed the message
#ifdef MESH_NET
    meshManager->sendParametersToSlaves(parameter.paramID, parameter.value);
#endif

    return true;
}

void MasterBoard::renderParameterMenu(bool forceClear)
{
    int selected = menuManager->getSelectedIndex();
    auto paramIDs = menuManager->getParametersForMenu(menuManager->getCurrentMenu());
    std::vector<ParameterDisplayItem> params;
    params.reserve(paramIDs.size());
    for (auto id : paramIDs)
    {
        ParameterDisplayItem item;
        item.name = getParameterName(id);
        if (isIntParameter(id))
        {
            auto p = getIntParameter(id);
            item.normalized = (float)(p.value - p.min) / (float)(p.max - p.min);
            item.valueText = std::to_string(p.value);
        }
        else if (isFloatParameter(id))
        {
            auto p = getFloatParameter(id);
            item.normalized = (p.value - p.min) / (p.max - p.min);
            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", p.value);
            item.valueText = buf;
        }
        else if (isBoolParameter(id))
        {
            auto p = getBoolParameter(id);
            item.normalized = p.value ? 1.0f : 0.0f;
            item.valueText = p.value ? "ON" : "OFF";
        }
        params.push_back(item);
    }
    auto menuPath = menuManager->getMenuPath(menuManager->getCurrentMenu(), MENU_MAIN);
    displayManager->displayParameterBars(params, selected, menuPath, forceClear);
}

#endif