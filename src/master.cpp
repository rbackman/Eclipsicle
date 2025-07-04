#ifdef MASTER_BOARD

#include <Arduino.h>
#include "serial.h"
#include "leds.h"
#include "sensors.h"
#ifdef USE_AUDIO
#include "audio.h"
#endif

#include <SPI.h>
#include "meshnet.h"
#include <ArduinoJson.h>
#include "audio.h"
#include "ledManager.h"

#define USE_DISPLAY 1

// Master Pin Layout
// 3v3                    Vin
// Gnd                    Gnd

// 15: SD_CS_PIN          13: BUTTON_LEFT
// 2:  SD_SCK             12: BUTTON_RIGHT
// 4:                     14: BUTTON_DOWN
// 16: RX BUTTON_TRIG     27: BUTTON_UP
// Tx:                    26: I2S_SD (Data In for INMP441)
// 5:  I2S_SCK            25:
// 18: I2S_WS             33: LED
// 19: SD_MISO            32: SLIDER5
// 21: OLED_SDA           35: SLIDER3
// rx0:                   34: SLIDER4
// tx0:                   39: SLIDER2
// 22: OLED_SCL           36: SLIDER1
// 23: SD_MOSI            37:

// I2S pins

#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

enum Menu_Mode
{

    MENU_MODE_MENU_CHOOSER,
    MENU_MODE_EDIT_MODE,
};

class Master : public ParameterManager
{
private:
    SerialManager *serialManager;
    SensorManager *sensorManager;

    MeshnetManager *meshManager;
#ifdef USE_AUDIO
    AudioManager *audioManager;
#endif
// StripState *leds;
#define NUM_LEDS 16
    CRGB ledsStrip[NUM_LEDS];
#define MASTER_LED_PIN 33

    JsonDocument doc;

    std::vector<std::string> menuText = {};
    // currentParent is the index of the current menu group
    MenuID currentMenu = MENU_IDLE;
    // selectedMenu is the index from 0 - length of activeMenus defined by menus that have the currentParent
    int selectedMenu = -1;
    int textOffset = 0;
    Menu_Mode menuMode = MENU_MODE_MENU_CHOOSER;
    int count = 0;
#ifdef USE_DISPLAY
    Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
    int lastDisplayUpdate = 0;
    int displayFrameRate = 10;
    std::string displayText = "";
    bool verticalAlignment = true; // Set this flag to true for vertical alignment, false for horizontal
#endif

public:
    Master() : ParameterManager("Master")
    {
        init();
    };

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
    };

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

                std::string name = getMenuName(amenu, 8);

                menuText.push_back(name);
            }
        }
        else if (menuMode == MENU_MODE_EDIT_MODE)
        {

            auto activeParams = getParametersForMenu(currentMenu);

            // const menu_item cmenu = menu[menuIndex];
            for (auto &param : activeParams)
            {
                auto name = getParameterName(param);
                if (name.size() > 6)
                {
                    name = name.substr(0, 6);
                }
                else if (name.size() < 6)
                {
                    name = name + std::string(6 - name.size(), ' ');
                }
                if (isBoolParameter(param))
                {
                    auto menuName = name + ":" + (getBool(param) ? "ON" : "OFF");
                    menuText.push_back(menuName.c_str());
                }
                else
                {
                    auto menuName = name + ":" + std::to_string(getValue(param));
                    menuText.push_back(menuName.c_str());
                }
            }

            display.setCursor(0, 0);
            int parent = currentMenu;
            // make sure menu path is at most 6 characters
            auto menupath = getMenuPath(currentMenu, MENU_PATTERNS);
            if (menupath.size() > 16)
            {
                menupath = menupath.substr(0, 16);
            }
            display.println(menupath.c_str());
        }

        for (int i = 0; i <= 8 && i < menuText.size(); i++)
        {
            int xpos, ypos;
            int yoffset = 0;
            std::string text = menuText[i];
            if (verticalAlignment)
            {
                // Vertical alignment
                xpos = (i / 6) * 64; // Switch column after every 4 items
                ypos = (i % 6) * 8;  // Y position based on item index within the column
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

            display.println(text.c_str());
        }
        display.display();
    };

    void init()
    {

        serialManager = new SerialManager(120, "Master");
        meshManager = new MeshnetManager();
        meshManager->connectSlaves();

        int value = getValue(PARAM_MASTER_LED_HUE);
        Serial.println("Starting Master Hue: " + String(value));

        // leds = new StripState(ledsStrip, LED_STATE_IDLE, 1, MASTER_LED_PIN, 0, false);
        FastLED.addLeds<NEOPIXEL, MASTER_LED_PIN>(ledsStrip, NUM_LEDS);
        FastLED.setBrightness(50);
        for (int i = 0; i < NUM_LEDS; i++)
        {
            int color = i * 255 / NUM_LEDS;
            ledsStrip[i] = CHSV(color, 255, 255);
        }
        FastLED.show();
#ifdef USE_AUDIO
        audioManager = new AudioManager();
        // audioManager->playTone(1500, 100, 5);
#endif

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
            {BUTTON, 13, BUTTON_LEFT},
            {BUTTON, 12, BUTTON_RIGHT},
            {BUTTON, 14, BUTTON_DOWN},
            {BUTTON, 27, BUTTON_UP},
            {BUTTON, 16, BUTTON_TRIGGER},
        });

        updateMenu();
    };

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
                            auto param = getIntParameter(paramID);

                            int val = lerp(0, 255, param.min, param.max, message.value);
                            // Serial.printf("Param[%d]  Slider: %d %s => %d\n", paramID, message.value, param.name.c_str(), val);

                            if (val != param.value)
                            {
                                if (isBoolParameter(paramID))
                                {
                                    setBool(paramID, val != 0);
                                }
                                else
                                {
                                    setValue(paramID, val);
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

#ifdef USE_AUDIO

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

#ifdef USE_AUDIO
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
#endif
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

                        auto availableMenus = getChildrenOfMenu(currentMenu);

                        if (selectedMenu >= availableMenus.size() || selectedMenu < 0)
                        {
                            if (availableMenus.size() == 0)
                            {
                                return false;
                            }
                            selectedMenu = 0;
                        }

                        currentMenu = availableMenus[selectedMenu];


                        std::string name = getMenuName(currentMenu, 32);

                        meshManager->sendStringToSlaves("menu:" + String(name.c_str()));

                        if (getChildrenOfMenu(currentMenu).size() == 0) // if there are zero root menus for a mode, then go to edit mode
                        {
                            selectedMenu = 0;

                            // Serial.print("Menu Selected " + name + " " + String(currentMenu));
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
    };

    void update()
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
            if (handleTextMessage(message.c_str()) == false)
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

        if (parameterChanged())
        {
            int masterHUE = getValue(PARAM_MASTER_LED_HUE);
            int masterBrightness = getValue(PARAM_MASTER_LED_BRIGHTNESS);
            int saturation = getValue(PARAM_MASTER_LED_SATURATION);

            int hue = masterHUE * 255 / 360;
            for (int i = 0; i < NUM_LEDS; i++)
            {

                ledsStrip[i] = CHSV(hue, saturation, masterBrightness);
            }

            Serial.printf("set led color hsv:%d => %d %d %d   rgb: %d %d %d\n", masterHUE, hue, saturation, masterBrightness, ledsStrip[0].r, ledsStrip[0].g, ledsStrip[0].b);
            FastLED.show();
        }

        //
    };
};

Master *master;

void setup()
{
    master = new Master();
    master->init();
}

void loop()
{
    master->update();
}
#endif