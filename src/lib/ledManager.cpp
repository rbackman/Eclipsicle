#ifdef USE_LEDS
#include "pins.h"
#include "stripState.h"
#include "ledManager.h"
#include "leds.h"
#include <string>
#include "shared_esp.h"

LEDManager::LEDManager() : ParameterManager("LEDManager", {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{
}
void LEDManager::initStrips()
{

    ledMatrix = new LedMatrix();

    // Serial.printf("Adding LEDManager for %s with %d strips\n", rig.name.c_str(), rig.strips.size());

    for (int i = 0; i < stripStates.size(); i++)
    {
        StripState *strip = stripStates[i];
        int stripIndex = strip->getStripIndex();

        switch (stripIndex)
        {
        case 0:

            FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(
                strip->leds, strip->getNumLEDS());
            break;
        case 1:

            FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(
                strip->leds, strip->getNumLEDS());
            break;
        case 2:

            FastLED.addLeds<LED_TYPE, LED_PIN_3, COLOR_ORDER>(
                strip->leds, strip->getNumLEDS());
            break;
        case 3:
            FastLED.addLeds<LED_TYPE, LED_PIN_4, COLOR_ORDER>(
                strip->leds, strip->getNumLEDS());
            break;

        default:
            Serial.println("Warning more strips than pins");
            break;
        }
    }
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
        stripStates[currentStrip]->setLEDRow(row);
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

void LEDManager::setAnimation(std::string animationName)
{
    // set the current animation for all strips

    for (int i = 0; i < stripStates.size(); i++)
    {
        stripStates[i]->setAnimationFromName(animationName);
    }
}

bool LEDManager::handleString(String command)
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
        Serial.printf("Setting current strip to %d\n", strip);
        setInt(PARAM_CURRENT_STRIP, strip);
        return true;
    }

    else
    {
        bool res = false;
        int currentStrip = getInt(PARAM_CURRENT_STRIP);

        for (int i = 0; i < stripStates.size(); i++)
        {
            // if (currentStrip == 0 || currentStrip == i + 1)
            // {

            if (stripStates[i]->handleTextMessage(std::string(command.c_str())))
            {

                res = true;
            }
            // }
        }

        return res;
    }
}

bool LEDManager::handleParameterMessage(parameter_message parameter)
{
    ParameterManager::handleParameterMessage(parameter);

    if (parameter.paramID == PARAM_BRIGHTNESS)
    {
        setBrightness(parameter.value);
    }
    if (parameter.paramID == PARAM_CURRENT_STRIP)
    {
        Serial.printf("Setting current strip to %d\n", parameter.value);
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
        if (currentStrip == -1)
        {
            for (int i = 0; i < stripStates.size(); i++)
            {
                stripStates[i]->handleParameterMessage(parameter);
            }
            return true;
        }
        Serial.println("update current strip " + String(currentStrip));
        if (currentStrip < 0 || currentStrip >= stripStates.size())
        {
            Serial.printf("Invalid current strip %d, valid range is 0 to %d\n", currentStrip, stripStates.size() - 1);
            return false;
        }
        return stripStates[currentStrip]->handleParameterMessage(parameter);
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

void LEDManager::toggleMode()
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip < 0 || currentStrip >= stripStates.size())
    {

        for (int i = 0; i < stripStates.size(); i++)
        {
            stripStates[i]->toggleMode();
        }
    }
    else
    {

        stripStates[currentStrip]->toggleMode();
    }
}
std::string LEDManager::getStripState(bool verbose)
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip < 0 || currentStrip >= stripStates.size())
    {

        return stripStates[0]->getStripState(verbose);
    }
    return stripStates[currentStrip]->getStripState(verbose);
}

std::string LEDManager::getStripsStateJson(bool verbose)
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip < 0 || currentStrip >= stripStates.size())
    {
        return getStripStateJson(stripStates[0], verbose);
    }
    return getStripStateJson(stripStates[currentStrip], verbose);
}

std::string LEDManager::getStripStateCompact(bool verbose)
{
    int currentStrip = getInt(PARAM_CURRENT_STRIP);
    if (currentStrip < 0 || currentStrip >= stripStates.size())

    {
        return stripStates[0]->getStripStateCompact();
    }
    return stripStates[currentStrip]->getStripStateCompact();
}

std::string LEDManager::getAnimationInfoJson()
{
    if (stripStates.empty())
    {
        return "{}";
    }
    auto stripState = stripStates[0];

    return getAnimationInfoJsonForStrip(stripState);
}

#endif