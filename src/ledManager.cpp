#ifdef USE_LEDS

#include "stripState.h"
#include "ledManager.h"
#include "leds.h"
#include "stripState.h"
#include <FastLED.h>

LEDManager::LEDManager(std::string slavename) : ParameterManager("LEDManager", {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{
    LEDRig rig;
    for (auto i : slaves)
    {
        if (i.name == slavename)
        {
            rig = i;
        }
    }
    if (rig.strips.size() == 0)
    {
        Serial.printf("No LED strips defined. add slave %s to shared.h \n", slavename.c_str());
        return;
    }
    ledMatrix = new LedMatrix();

    for (int i = 0; i < rig.strips.size(); i++)
    {
        LEDParams params = rig.strips[i];
        StripState *strip = new StripState(params.startState, params.numLEDS, params.stripIndex, params.reverse);
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
    setValue(PARAM_BRIGHTNESS, 50);
}
LEDManager::LEDManager(std::string name, std::vector<StripState *> strips) : ParameterManager(name.c_str(), {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{
    ledMatrix = new LedMatrix();
    for (int i = 0; i < strips.size(); i++) // Changed sizeof(strips) to strips.size()
    {
        stripStates.push_back(strips[i]);                                                // Changed &strips[i] to strips[i]
        Serial.printf("Adding strip %d with %d LEDs\n", i + 1, strips[i]->getNumLEDS()); // Changed strips[i].getNumLEDS() to strips[i]->getNumLEDS()
    }
    initStrips();
    setValue(PARAM_CURRENT_STRIP, 1);
}
int LEDManager::getCurrentStrip()
{
    return getValue(PARAM_CURRENT_STRIP);
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

    int currentStrip = getValue(PARAM_CURRENT_STRIP);
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

        setValue(PARAM_CURRENT_STRIP, strip);
        return true;
    }
    else
    {
        bool res = false;
        int currentStrip = getValue(PARAM_CURRENT_STRIP);

        for (int i = 0; i < stripStates.size(); i++)
        {
            if (currentStrip == 0 || currentStrip == i + 1)
            {
                if (stripStates[i]->respondToText(command))
                {
                    res = true;
                }
            }
        }

        return res;
    }
}

void LEDManager::respondToParameterMessage(parameter_message parameter)
{
    ParameterManager::respondToParameterMessage(parameter);
    if (parameter.paramID == PARAM_BRIGHTNESS)
    {
        setBrightness(parameter.value);
    }
    else if (parameter.paramID == PARAM_CURRENT_STRIP)
    {
        setValue(PARAM_CURRENT_STRIP, parameter.value);
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
    }
    else
    {
        int currentStrip = getValue(PARAM_CURRENT_STRIP);
        // Serial.println("update current strip " + String(currentStrip));
        for (int i = 0; i < stripStates.size(); i++)
        {
            if (currentStrip == 0 || currentStrip == i + 1)
            {
                stripStates[i]->respondToParameterMessage(parameter);
            }
        }
    }
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
    int currentStrip = getValue(PARAM_CURRENT_STRIP);
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
    int currentStrip = getValue(PARAM_CURRENT_STRIP);
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
    int currentStrip = getValue(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        return stripStates[0]->getStripState();
    }
    return stripStates[currentStrip - 1]->getStripState();
}

#endif