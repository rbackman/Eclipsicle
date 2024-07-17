#ifdef MASTER_BOARD

#include <Arduino.h>
#include "serial.h"
#include "leds.h"
#include "sensors.h"
// #include "sound.h"
// #include <SD.h>
#include <SPI.h>
#include "meshnet.h"
#include <ArduinoJson.h>
#include "audio.h"
#include "ledManager.h"

#define USE_DISPLAY 1

// Master Pin Layout
// 3v3               Vin
// Gnd               Gnd
// 15: SD_CS_PIN         13: BUTTON_LEFT
// 2:  SD_SCK             12: BUTTON_RIGHT
// 4:                14: BUTTON_DOWN
// 16:BUTTON_TRIGGER 27: BUTTON_UP
// Tx:               26: I2S_SD
// 5:   I2S_SCK       25: AUDIO_OUT_PIN  DAC
// 18:  I2S_WS        33: LED
// 19:  SD_MISO        32: SLIDER5
// 21:               35: SLIDER3
// 22:               34: SLIDER4
// 23:  SD_MOSI       39:  SLIDER2
// 3:                36:  SLIDER1
// 1:                EN:
// 3v3               3v3
// Gnd               Gnd

#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

const std::vector<std::string> slaves = {"Eclipsicle"}; //,"Go", "Slave1"};

SerialManager *serialManager;
SensorManager *sensorManager;

MeshnetManager *meshManager;
AudioManager *audioManager;
StripState *leds;
CRGB ledsStrip[1];
#define MASTER_LED_PIN 33
// SoundManager *soundManager;
JsonDocument doc;

std::vector<std::string> menuText = {};

ParameterManager *parameterManager;

enum Menu_Mode
{

    MENU_MODE_MENU_CHOOSER,
    MENU_MODE_EDIT_MODE,
};

// currentParent is the index of the current menu group
MenuID currentMenu = MENU_AUDIO;
// selectedMenu is the index from 0 - length of activeMenus defined by menus that have the currentParent
int selectedMenu = -1;
int textOffset = 0;
Menu_Mode menuMode = MENU_MODE_MENU_CHOOSER;

int numMenus()
{

    if (menuMode == MENU_MODE_MENU_CHOOSER)
    {

        return getChildrenOfMenu(currentMenu).size();
    }
    else if (menuMode == MENU_MODE_EDIT_MODE)
    {
        return getParametersForMenu(currentMenu).size();
    }
    return 0;
}

#ifdef USE_DISPLAY
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
int lastDisplayUpdate = 0;
int displayFrameRate = 10;
std::string displayText = "";
bool verticalAlignment = true; // Set this flag to true for vertical alignment, false for horizontal

void updateMenu()
{
    display.clearDisplay();
    if (currentMenu == MENU_IDLE)
    {
        display.setCursor(0, 0);
        display.println(".");
        display.display();
        return;
    }

    menuText.clear();

    if (menuMode == MENU_MODE_MENU_CHOOSER)
    {
        display.setRotation(0);

        auto availableMenus = getChildrenOfMenu(currentMenu);

        for (const auto &amenu : availableMenus)
        {

            std::string name = getMenuName(amenu);

            menuText.push_back(name);
        }
    }
    else if (menuMode == MENU_MODE_EDIT_MODE)
    {

        auto activeParams = getParametersForMenu(currentMenu);

        // const menu_item cmenu = menu[menuIndex];
        for (auto &param : activeParams)
        {
            auto menuName = parameterManager->getParameterName(param) + ":" + std::to_string(parameterManager->getValue(param));
            menuText.push_back(menuName.c_str());
        }

        display.setCursor(0, 0);
        int parent = currentMenu;
        display.println(getMenuPath(currentMenu, MENU_PATTERNS).c_str());
    }

    for (int i = 0; i <= 8 && i < menuText.size(); i++)
    {
        int xpos, ypos;
        int yoffset = 0;
        std::string text = menuText[i];
        if (verticalAlignment)
        {
            // Vertical alignment
            xpos = (i / 4) * 64; // Switch column after every 4 items
            ypos = (i % 4) * 8;  // Y position based on item index within the column
        }
        else
        {
            // Horizontal alignment
            xpos = (i % 2) * 64; // Two columns per row
            ypos = (i / 2) * 8;  // Four rows
            yoffset = 8;
        }
        display.setCursor(xpos, ypos + yoffset);

        // Check if the text is on the current line

        bool isCurrent = false;
        if (menuMode == MENU_MODE_MENU_CHOOSER)
        {
            isCurrent = i == selectedMenu;
        }
        else if (menuMode == MENU_MODE_EDIT_MODE)
        {
            isCurrent = false; // i == currentLine;
        }

        if (isCurrent)
        {
            display.fillRect(xpos, ypos + yoffset, 64, 8, WHITE); // Draw a white rectangle for highlighting
            display.setTextColor(BLACK);                          // Set text color to black for inversion
        }
        else
        {
            display.fillRect(xpos, ypos + yoffset, 64, 8, BLACK); // Draw a black rectangle for normal text
            display.setTextColor(WHITE);                          // Set text color to white for normal text
        }
        // Serial.print("Menu Text: => ");
        // Serial.println(text.c_str());

        // display.setTextColor(WHITE);
        display.println(text.c_str());
    }
    display.display();
}

#endif

void setup()
{

    serialManager = new SerialManager(120, "Master");
    meshManager = new MeshnetManager();
    meshManager->connectSlaves(slaves);
    parameterManager = new ParameterManager("Master");

    audioManager = new AudioManager();

    leds = new StripState(parameterManager, ledsStrip, LED_STATE_RAINBOW, 1, MASTER_LED_PIN, 0, true);
    FastLED.addLeds<NEOPIXEL, MASTER_LED_PIN>(ledsStrip, LEDS_STRIP_1);
    FastLED.setBrightness(50);

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
    sensorManager = new SensorManager({
        {SLIDER, 36, SLIDER1, true},
        {SLIDER, 39, SLIDER2, true},
        {SLIDER, 35, SLIDER3, true},
        {SLIDER, 34, SLIDER4, true},
        {SLIDER, 32, SLIDER5, true},
        {DIAL, 33, DIAL1},
        {BUTTON, 13, BUTTON_LEFT},
        {BUTTON, 12, BUTTON_RIGHT},
        {BUTTON, 14, BUTTON_DOWN},
        {BUTTON, 27, BUTTON_UP},
        {BUTTON, 16, BUTTON_TRIGGER},
    });

    updateMenu();
}

bool processSensorMessage(sensor_message message)
{
    bool used = false;
    if (message.type != MESSAGE_TYPE_SENSOR)
    {
        Serial.println("Eror Not a sensor message");
        return false;
    }
    if (message.sensorType == SLIDER)
    {

        if (menuMode == MENU_MODE_EDIT_MODE)
        {

            auto activeparams = getParametersForMenu(currentMenu);
            try
            {
                for (int paramIndex = 0; paramIndex < activeparams.size(); paramIndex++)
                {

                    if ((message.sensorId == SLIDER1 && paramIndex == 0) ||
                        (message.sensorId == SLIDER2 && paramIndex == 1) ||
                        (message.sensorId == SLIDER3 && paramIndex == 2) ||
                        (message.sensorId == SLIDER4 && paramIndex == 3) ||
                        (message.sensorId == SLIDER5 && paramIndex == 4))
                    {
             
                        auto paramID = activeparams[paramIndex];
                        auto param = parameterManager->getIntParameter(paramID);

                        int val = lerp(0, 255, param.min, param.max, message.value);
                        // Serial.printf("Param[%d]  Slider: %d %s => %d\n", paramID, message.value, param.name.c_str(), val);

                        if (val != param.value)
                        {
                            if (parameterManager->isBoolParameter(paramID))
                            {
                                parameterManager->setBool(paramID, val != 0);
                            }
                            else
                            {
                                parameterManager->setValue(paramID, val);
                            }

                            meshManager->sendParametersToSlaves(paramID, val);
                            used = true;
                        }
                    }

                    if (used)
                    {
                        updateMenu();
                        return true;
                    }
                }
            }
            catch (const std::exception &e)
            {
                Serial.println("Error in Slider Processing");
            }
        }
    }
    else if (message.sensorType == DIAL)
    {
    }
    else if (message.sensorType == BUTTON)
    {
        int nummenus = numMenus();

        if (message.sensorId == BUTTON_TRIGGER)
        {
            audioManager->playTone(1000, 100, 5);
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

        if (message.value == 1)
        {

            if (menuMode == MENU_MODE_EDIT_MODE)
            {

                if (message.sensorId == BUTTON_LEFT)
                {

                    std::string menuName = getMenuName(currentMenu);
                    currentMenu = getParentMenu(currentMenu);
                    Serial.print("Leaving parameter Mode ");
                    Serial.println(menuName.c_str());
                    verticalAlignment = true;
                    menuMode = MENU_MODE_MENU_CHOOSER;
                    used = true;
                }
                if (message.sensorId == BUTTON_TRIGGER)
                {

                    if (currentMenu == MENU_AUDIO)
                    {
                        if (!audioManager->isRecordingAudio())
                        {
                            Serial.println("Recording Audio");
                            audioManager->record();
                        }
                        else
                        {
                            Serial.println("Playing Audio");
                            audioManager->stop();
                        }
                    }
                }
                if (message.sensorId == BUTTON_RIGHT)
                {
                    audioManager->play();
                }
                else if (message.sensorId == BUTTON_UP)
                {
                    audioManager->debugAudio();
                }
            }
            else if (menuMode == MENU_MODE_MENU_CHOOSER)
            {
                if (message.sensorId == BUTTON_UP)
                {
                    // up button
                    selectedMenu--;
                    if (selectedMenu < 0)
                    {
                        selectedMenu = nummenus - 1;
                        if (selectedMenu >= 4)
                        {
                            textOffset = selectedMenu - 4;
                        }
                        else
                        {
                            textOffset = 0;
                        }
                    }

                    auto smenus = getChildrenOfMenu(currentMenu);
                    if (selectedMenu < 0)
                    {
                        selectedMenu = smenus.size() - 1;
                    }

                    auto menu = smenus[selectedMenu];

                    used = true;
                }

                if (message.sensorId == BUTTON_DOWN)
                {

                    // down button
                    selectedMenu++;
                    if (selectedMenu >= 4)
                    {
                        textOffset = selectedMenu - 4;
                    }
                    else
                    {
                        textOffset = 0;
                    }

                    auto smenus = getChildrenOfMenu(currentMenu);
                    if (selectedMenu >= smenus.size())
                    {
                        selectedMenu = 0;
                    }
                    auto menu = smenus[selectedMenu];
                    Serial.printf("Selected Menu %d  %d %s\n", selectedMenu, menu, getMenuName(menu));

                    used = true;
                }
                if (message.sensorId == BUTTON_LEFT)
                {

                    // back button
                    Serial.println("Back Button");
                    if (currentMenu != MENU_IDLE)
                    {
                        currentMenu = getParentMenu(currentMenu);
                    }

                    used = true;
                }

                if (message.sensorId == BUTTON_RIGHT)
                {

                    Serial.println("Last Menu Selected " + String(selectedMenu) + " " + String(getMenuName(currentMenu)));
                    auto availableMenus = getChildrenOfMenu(currentMenu);
                    Serial.println("Available Menus " + String(availableMenus.size()));
                    for (size_t i = 0; i < availableMenus.size(); i++)
                    {
                        Serial.println("Menu " + String(i) + " " + String(getMenuName(availableMenus[i])));
                    }

                    if (selectedMenu >= availableMenus.size() || selectedMenu < 0)
                    {
                        if (availableMenus.size() == 0)
                        {
                            Serial.println("Selected Menu out of range" + String(selectedMenu) + " " + String(availableMenus.size()));
                            Serial.println("current Menu" + String(currentMenu) + " " + getMenuName(currentMenu));
                            return false;
                        }
                        selectedMenu = 0;
                    }

                    currentMenu = availableMenus[selectedMenu];

                    Serial.printf("Selected Menu %d  %d %s\n", selectedMenu, currentMenu, getMenuName(currentMenu));

                    std::string name = getMenuName(currentMenu);

                    meshManager->sendStringToSlaves("menu:" + String(name.c_str()));

                    if (getChildrenOfMenu(currentMenu).size() == 0) // if there are zero root menus for a mode, then go to edit mode
                    {
                        selectedMenu = 0;
                        String name = getMenuName(currentMenu);
                        Serial.print("Menu Selected " + name + " " + String(currentMenu));
                        verticalAlignment = false;
                        menuMode = MENU_MODE_EDIT_MODE;
                    }
                }
                if (message.sensorId == BUTTON_TRIGGER)
                {
                    Serial.println("Trigger Button");
                }
                used = true;
            }
            else
            {
                Serial.println("Not Handle Menu Mode" + String(menuMode) + " " + String(getSensorName(message.sensorId)));
            }

            if (used)
            {
                updateMenu();
                return true;
            }
        }
    }
    return false;
}

void loop()
{
    audioManager->update();
    leds->update();
    sensorManager->updateSensors();
    serialManager->updateSerial();
    // soundManager->update();

    while (sensorManager->messageAvailable())
    {

        sensor_message message = sensorManager->getNextMessage();

        if (!processSensorMessage(message))
        {

            // Serial.println("Send Sensor Message to slaves: " + message.name + " " + String(message.value));
            // meshManager->sendSensorCommandToSlaves(message);
        }
    }

    int curentTime = millis();
    if (curentTime - lastDisplayUpdate > 1000 / 60) // 1000 / displayFrameRate)
    {
        lastDisplayUpdate = curentTime;

        // #ifdef USE_DISPLAY
        //         display.clearDisplay();
        //         display.setCursor(0, 0);

        //         display.print("Accel ");

        //         display.display();

        // #endif
    }

    // if (sensorManager->buttonPressed("Button4"))
    // {

    //     meshManager->sendStringToSlaves("volume:" + String(soundManager->getSoundLevel()));
    // }

    if (serialManager->stringAvailable())
    {
        String message = serialManager->readString();
        //   Serial.print("Send Serial Message: ");

        // Serial.print("Send Serial Message: " + message);
        meshManager->sendStringToSlaves(message);
    }
    if (serialManager->jsonAvailable())
    {

        if (
            serialManager->readJson(doc))
        {

            if (doc.containsKey("pixleByte"))
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
                Serial.println("Doesn't contain pixleByte");
                deserializeJson(doc, Serial);
            }

            serialManager->clearBuffer();
        }
    }
}

#endif