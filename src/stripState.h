#pragma once

#ifdef USE_LEDS
#include "FastLED.h"
#include "shared.h"
#include "leds.h"

#include "parameterManager.h"
struct Particle
{
    bool active = false;
    int life = -1;
    float position = 0;
    float maxSpeed = 1.5;
    float velocity = 0.2;
    float acceleration = 0.5;

    float fade = 0.9;
    int randomDrift = 5;

    int hueStart;
    int hueEnd;
    int brightness;
    int width;
};

struct SliderParams
{
    int position;
    int width;
    int hueshift;
    int repeat;
    bool useGravity;
};
class StripState : public ParameterManager
{

private:
    int stripIndex;
    int gravityPosition = 0;
    int numLEDS = 128;
    bool invertLEDs = false;

    float scrollPos = 0;

    LED_STATE ledState = LED_STATE_IDLE;
    Particle particles[10];

public:
    CRGB *leds;
    StripState(LED_STATE state, const int numLEDS, const int LED_PIN, int STRIP_INDEX, bool invert);

    void setNumLEDS(int num)
    {
        numLEDS = num;
    }
    int getNumLEDS()
    {
        return numLEDS;
    }
    void setGravityPosition(float position)
    {
        gravityPosition = (int)(position * numLEDS);
    }
    void fadeParticleTail(int position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction);

    void spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life)
    {

        bool used = false;
        for (int i = 0; i < 10; i++)
        {
            if (!particles[i].active)
            {
                used = true;
                particles[i].active = true;
                particles[i].position = position;
                particles[i].velocity = velocity;
                particles[i].hueStart = hueStart;
                particles[i].hueEnd = hueEnd;
                particles[i].brightness = brightness;
                particles[i].width = width;
                particles[i].life = life;

                // if (isVerbose())
                // {
                // Serial.printf("\nSpawned particle\n  position x: %d\n velocity: %f \nhueStart: %d \nhueEnd: %d \nbrightness: %d \nwidth: %d \nlife: %d", position, velocity, hueStart, hueEnd, brightness, width, life);
                // }

                return;
            }
        }
        if (!used)
        {
            Serial.println("No free particles");
        }
    }

    void spawnParticle();
    void updateRandomParticles();
    void updateParticles();

    void setLEDRow(LedRow ledRow)
    {

        for (int i = 0; i < numLEDS; i++)
        {
            int r = ledRow[i].r;
            int g = ledRow[i].g;
            int b = ledRow[i].b;
            setPixel(i, r, g, b);
        }
    }
    void setLEDState(LED_STATE state)
    {
        ledState = state;
    }

    void setAll(CRGB color);
    void setAll(led color);
    bool respondToText(String command);

    void clearPixels();
    void clearPixel(int index);
    void setPixel(int index, led color);
    void setPixel(int index, int r, int g, int b);

    void toggleMode();

    void update();
    String getStripState();
    void respondToParameterMessage(parameter_message parameter);
};

#endif