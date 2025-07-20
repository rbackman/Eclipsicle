#ifdef MASTER_BOARD

#include <Arduino.h>

#ifdef USE_AUDIO
#include "audio.h"
#endif

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

        {0x98, 0x88, 0xE0, 0x03, 0xFF, 0x58}, // Slave1 98:88:E0:03:FF:58

    };
    meshManager->connectSlaves(slaves);

#ifdef USE_AUDIO
    audioManager = new AudioManager();
    // audioManager->playTone(1500, 100, 5);
#endif

    spiBus.begin(SCL_PIN, DOUT_PIN, SDA_PIN, MCP_CS);
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
    audioManager->update();

    auto decibels = audioManager->getDecibel();
    if (decibels > 0)
    {
        Serial.println("Decibels: " + String(decibels));
    }
#endif
    sensorManager->updateSensors();
    serialManager->updateSerial();

    if (menuManager->isMenuChanged())
    {
        auto menuItems = menuManager->getMenuItems();
        displayManager->displayMenu(menuItems);
    }
    if (menuManager->messageAvailable())
    {
        auto message = menuManager->getMessage();
        Serial.println("sending to slaves  : " + String(message.paramID) + " " + String(message.value));
        meshManager->sendParametersToSlaves(message.paramID, message.value);
    }
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

    if (!ParameterManager::handleParameterMessage(parameter))
    {
        Serial.println("sending to slaves");
        // Handle the case where no manager processed the message
#ifdef MESH_NET
        meshManager->sendParametersToSlaves(parameter.paramID, parameter.value);
#endif
    }
    return true;
}

#endif