#ifdef USE_LEDS

#include "stripState.h"
#include "ledManager.h"
#include "leds.h"
#include "stripState.h"
#include <FastLED.h>

const std::vector<LEDRig> slaves = {
    {
        "Eclipsicle",
        {0x40, 0x91, 0x51, 0xFB, 0xB7, 0x48},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},
            {200, 1, ANIMATION_TYPE_SLIDER, false},

        },
    },
    {
        "Tesseratica",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},

        },
    },
    {
        "tradeday",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {100, 0, ANIMATION_TYPE_IDLE, false},
        },
    },
    {
        "simpled",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {164, 0, ANIMATION_TYPE_PARTICLES, false},

        },
    },
    {
        "Bike",
        {0xD0, 0xEF, 0x76, 0x58, 0x45, 0xB4},
        {
            {48, 0, ANIMATION_TYPE_SLIDER, false},
            {48, 1, ANIMATION_TYPE_SLIDER, false},
            {18, 2, ANIMATION_TYPE_SLIDER, false},
        },
    },
    {

        "Spinner",
        {0xD0, 0xEF, 0x76, 0x58, 0x45, 0xB4},
        {
            {280, 0, ANIMATION_TYPE_IDLE, false},
            {280, 1, ANIMATION_TYPE_IDLE, false},
        },
    },

    {
        "Bike",
        {0xD0, 0xEF, 0x76, 0x57, 0x3F, 0xA0},
        {
            {100, 0, ANIMATION_TYPE_IDLE, false},
            {100, 1, ANIMATION_TYPE_IDLE, false},
            {100, 2, ANIMATION_TYPE_IDLE, false},
        },
    }};

LEDManager::LEDManager(std::string slavename) : ParameterManager("LEDManager", {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{

    LEDRig rig;
    for (int i = 0; i < slaves.size(); i++)
    {
        LEDRig slave = slaves[i];

        if (slave.name.compare(slavename) == 0)
        {
            rig = slave;
        }
    }
    if (rig.name == "")
    {
        Serial.printf("No LED rig found with name %s\n", slavename.c_str());
        return;
    }
    if (rig.strips.size() == 0)
    {
        Serial.printf("No LED strips defined. add slave %s to shared.h \n", slavename.c_str());
        return;
    }
    ledMatrix = new LedMatrix();

    // Serial.printf("Adding LEDManager for %s with %d strips\n", rig.name.c_str(), rig.strips.size());
    for (int i = 0; i < rig.strips.size(); i++)
    {
        LEDParams params = rig.strips[i];
        StripState *strip = new StripState(LED_STATE_SINGLE_ANIMATION, params.numLEDS, params.stripIndex, params.reverse);

        strip->setAnimation(params.startState);
        stripStates.push_back(strip);
    }
    initStrips();
}

void LEDManager::initStrips()
{
    for (int i = 0; i < stripStates.size(); i++)
    {
        StripState *strip = stripStates[i];
        Serial.printf("Adding strip %d with %d LEDs\n", i + 1, strip->getNumLEDS());
        switch (i)
        {
        case 0:
            Serial.println("Adding strip 1");
            FastLED.addLeds<NEOPIXEL, LED_PIN_1>(strip->leds, strip->getNumLEDS());
            break;
        case 1:
            Serial.println("Adding strip 2");
            FastLED.addLeds<NEOPIXEL, LED_PIN_2>(strip->leds, strip->getNumLEDS());
            break;
        case 2:
            Serial.println("Adding strip 3");
            FastLED.addLeds<NEOPIXEL, LED_PIN_3>(strip->leds, strip->getNumLEDS());
            break;
        case 3:
            FastLED.addLeds<NEOPIXEL, LED_PIN_4>(strip->leds, strip->getNumLEDS());
            break;

        default:
            Serial.println("Warning more strips than pins");
            break;
        }
    }
    setInt(PARAM_BRIGHTNESS, 50);
}
LEDManager::LEDManager(std::string name, std::vector<StripState *> strips) : ParameterManager(name.c_str(), {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{
    Serial.printf("Creating LEDManager with %d strips\n", strips.size());
    stripStates = strips; // Changed &strips to strips
    if (strips.size() == 0)
    {
        Serial.println("No strips defined");
        return;
    }
    ledMatrix = new LedMatrix();
    for (int i = 0; i < strips.size(); i++) // Changed sizeof(strips) to strips.size()
    {
        stripStates.push_back(strips[i]);                                                // Changed &strips[i] to strips[i]
        Serial.printf("Adding strip %d with %d LEDs\n", i + 1, strips[i]->getNumLEDS()); // Changed strips[i].getNumLEDS() to strips[i]->getNumLEDS()
    }
    initStrips();
    setInt(PARAM_CURRENT_STRIP, 1);
}
int LEDManager::getCurrentStrip()
{
    return getInt(PARAM_CURRENT_STRIP);
}
void LEDManager::setLEDImage(image_message msg)
{

    std::vector<led> row = (*ledMatrix)[msg.row]; // Dereference the pointer first

    ByteRow rleRow = base64Decode(msg.pixleBytes, msg.numBytes);

    if (isVerbose())
    {
        Serial.println("\nDecoded row: ");
        printBytes(rleRow);
        Serial.println("\n");
    }
    if (row.size() != MAX_LEDS_PER_STRIP)
    {
        row.clear();
        row.resize(MAX_LEDS_PER_STRIP);
    }

    decodeRLE(rleRow, row);

    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        for (int i = 0; i < stripStates.size(); i++)
        {
            stripStates[i]->setLEDRow(row);
        }
    }
    else
    {
        stripStates[currentStrip - 1]->setLEDRow(row);
    }
}

void LEDManager::setLED(int ledIndex, led color)
{
    // set all leds to a color
    for (int i = 0; i < stripStates.size(); i++)
    {
        stripStates[i]->setPixel(ledIndex, color);
    }
}

bool LEDManager::handleLEDCommand(String command)
{

    if (command.startsWith("brightness"))
    {
        int sensorVal = command.substring(11).toInt();

        int brightness = map(sensorVal, 0, 255, 0, 255) / 255.0;

        setBrightness(brightness);
        return true;
    }

    else if (command.startsWith("strip:"))
    {
        int strip = command.substring(6).toInt();

        setInt(PARAM_CURRENT_STRIP, strip);
        return true;
    }
    else
    {
        bool res = false;
        int currentStrip = getInt(PARAM_CURRENT_STRIP);
        if (isVerbose())
        {
            Serial.printf("\nLEDManager handling command %s for strip %d of %d\n", command.c_str(), currentStrip, stripStates.size());
        }
        for (int i = 0; i < stripStates.size(); i++)
        {
            if (currentStrip == 0 || currentStrip == i + 1)
            {
                // Serial.printf("stripStates handling command %s for strip %d\n", command.c_str(), i + 1);
                if (stripStates[i]->respondToText(command))
                {
                    res = true;
                }
            }
        }
        if (!res)
        {
            Serial.printf("LEDManager command %s not handled for %d strips\n", command.c_str(), stripStates.size());
        }

        return res;
    }
}

bool LEDManager::respondToParameterMessage(parameter_message parameter)
{
    ParameterManager::respondToParameterMessage(parameter);
    Serial.printf("LEDManager responding to parameter message %d %d %d\n", parameter.type, parameter.paramID, parameter.value);
    if (parameter.paramID == PARAM_BRIGHTNESS)
    {
        setBrightness(parameter.value);
        return true;
    }
    else if (parameter.paramID == PARAM_CURRENT_STRIP)
    {
        setInt(PARAM_CURRENT_STRIP, parameter.value);
        for (int i = 0; i < stripStates.size(); i++)
        {
            if (parameter.value == 0 || parameter.value == i + 1)
            {
                stripStates[i]->isActive = true;
            }
            else
            {
                stripStates[i]->isActive = false;
            }
        }
        return true;
    }
    else
    {
        int currentStrip = getInt(PARAM_CURRENT_STRIP);
        // Serial.println("update current strip " + String(currentStrip));
        for (int i = 0; i < stripStates.size(); i++)
        {
            if (currentStrip == i)
            {
                return stripStates[i]->respondToParameterMessage(parameter);
            }
        }
        Serial.printf("LEDManager does not have strip %d\n", currentStrip);
    }
    return false;
}

void LEDManager::setBrightness(int brightness)
{

    // setValue(PARAM_BRIGHTNESS, brightness);
    FastLED.setBrightness(brightness);
}
void LEDManager::setAll(led color)
{
    // set all leds to a color
    for (int i = 0; i < stripStates.size(); i++)
    {
        stripStates[i]->setAll(color);
    }
}

void LEDManager::update()
{
    int currentTime = millis();
    int deltaTime = currentTime - lastUpdate;

    if (deltaTime < 1000 / fps)
    {
        return;
    }

    lastUpdate = currentTime;

    for (int i = 0; i < stripStates.size(); i++)
    {
        stripStates[i]->update();
    }
    // for (int i = 0; i < stripStates.size(); i++)
    // {

    //     Serial.printf("\n  strip %d led array ", i);
    //     // print the first few leds of strip 1
    //     for (int j = 0; j < 5; j++)
    //     {
    //         Serial.print(stripStates[i]->leds[j].r);
    //         Serial.print(" ");
    //         Serial.print(stripStates[i]->leds[j].g);
    //         Serial.print(" ");
    //         Serial.print(stripStates[i]->leds[j].b);
    //         Serial.print(" ,");
    //     }
    // }
    FastLED.show();
}

void LEDManager::setGravityPosition(float position)
{
    // gravity position is the LED index that is the bottom of the strip according to the accelerometer
    gravityPosition = position;
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        for (int i = 0; i < stripStates.size(); i++)
        {
            stripStates[i]->setGravityPosition(position);
        }
    }
    else
    {
        stripStates[currentStrip - 1]->setGravityPosition(position);
    }
}

void LEDManager::toggleMode()
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        for (int i = 0; i < stripStates.size(); i++)
        {
            stripStates[i]->toggleMode();
        }
    }
    else
    {
        stripStates[currentStrip - 1]->toggleMode();
    }
}
String LEDManager::getStripState()
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        return stripStates[0]->getStripState();
    }
    return stripStates[currentStrip - 1]->getStripState();
}

#endif