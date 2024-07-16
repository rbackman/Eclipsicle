#include "stripState.h"
#include "ledManager.h"
#include "leds.h"
#include "stripState.h"
#include <FastLED.h>

CRGB ledsStrip1[LEDS_STRIP_1];
CRGB ledsStrip2[LEDS_STRIP_2];

LEDManager::LEDManager()
{
    ledMatrix = new LedMatrix();

    stripStates[0] = new StripState(ledsStrip1, LED_STATE_PARTICLES, LEDS_STRIP_1, LED_PIN_1, 0, true);
    stripStates[1] = new StripState(ledsStrip2, LED_STATE_PARTICLES, LEDS_STRIP_2, LED_PIN_2, 1, false);

    FastLED.addLeds<NEOPIXEL, LED_PIN_1>(ledsStrip1, LEDS_STRIP_1);
    FastLED.addLeds<NEOPIXEL, LED_PIN_2>(ledsStrip2, LEDS_STRIP_2);
    setBrightness(50);
}
void LEDManager::setCurrentStrip(int strip)
{
    currentStrip = strip;
}

void LEDManager::setLEDImage(image_message msg)
{

    if (msg.size != msg.row >= ledMatrix->size())
    {
        Serial.println("Error: ledMatrix size does not match message size");

        // ledMatrix->clear();          // Arrow operator again
        // ledMatrix->resize(msg.size); // And again
        // for (int i = 0; i < msg.size; i++)
        // {
        //     (*ledMatrix)[i].resize(MAX_LEDS_PER_STRIP); // Here, we dereference the pointer and then use the array index operator.
        // }
    }
    else
    {
        // Serial.println("ledMatrix size matches " + String(msg.size));
    }

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

    // ledMatrix->at(msg.row) = row;

    StripState *s = stripStates[currentStrip];
    s->setLEDRow(row);
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
        setCurrentStrip(strip);
        return true;
    }
    else
    {
        bool res = false;

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


void LEDManager::respondToParameter(parameter_message parameter)
{
    if (currentStrip == 0)
    {
        for (int i = 0; i < NUM_STRIPS; i++)
        {
            stripStates[i]->respondToParameter(parameter);
        }
    }
    else
    {
        stripStates[currentStrip - 1]->respondToParameter(parameter);
    }
}
void LEDManager::setBrightness(int brightness)
{

    this->brightness = brightness;
    FastLED.setBrightness(brightness);
}
void LEDManager::setAll(led color)
{
    // You'll need to set all the pixels to val
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        StripState *s = stripStates[i];
        for (int j = 0; j < s->getNumLEDS(); j++)
        {
            stripStates[i]->setPixel(j, color);
        }
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
    gravityPosition = position;
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
    if (currentStrip == 0)
    {
        return stripStates[0]->getStripState();
    }
    return stripStates[currentStrip - 1]->getStripState();
}