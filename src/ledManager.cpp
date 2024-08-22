#ifdef USE_LEDS

#include "stripState.h"
#include "ledManager.h"
#include "leds.h"
#include "stripState.h"
#include <FastLED.h>

CRGB ledsStrip1[LEDS_STRIP_1];
CRGB ledsStrip2[LEDS_STRIP_2];

// std::vector<CRGB> ledStrips ;

LEDManager::LEDManager() : ParameterManager("LEDManager", {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE})
{

    ledMatrix = new LedMatrix();

    stripStates[0] = new StripState(ledsStrip1, LED_STATE_PARTICLES, LEDS_STRIP_1, LED_PIN_1, 0, true);
    stripStates[1] = new StripState(ledsStrip2, LED_STATE_PARTICLES, LEDS_STRIP_2, LED_PIN_2, 1, false);

    FastLED.addLeds<NEOPIXEL, LED_PIN_1>(ledsStrip1, LEDS_STRIP_1);
    FastLED.addLeds<NEOPIXEL, LED_PIN_2>(ledsStrip2, LEDS_STRIP_2);
    // setValue(PARAM_BRIGHTNESS, 50);
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
        for (int i = 0; i < NUM_STRIPS; i++)
        {
            stripStates[i]->setLEDRow(row);
        }
    }
    else
    {
        stripStates[currentStrip - 1]->setLEDRow(row);
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

        for (int i = 0; i < NUM_STRIPS; i++)
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

    else
    {
        int currentStrip = getValue(PARAM_CURRENT_STRIP);
        Serial.println("update current strip " + String(currentStrip));
        for (int i = 0; i < NUM_STRIPS; i++)
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
    for (int i = 0; i < NUM_STRIPS; i++)
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

    for (int i = 0; i < NUM_STRIPS; i++)
    {
        stripStates[i]->update();
    }

    FastLED.show();
}

void LEDManager::setGravityPosition(float position)
{
    // gravity position is the LED index that is the bottom of the strip according to the accelerometer
    gravityPosition = position;
    int currentStrip = getValue(PARAM_CURRENT_STRIP);
    if (currentStrip == 0)
    {
        for (int i = 0; i < NUM_STRIPS; i++)
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
        for (int i = 0; i < NUM_STRIPS; i++)
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