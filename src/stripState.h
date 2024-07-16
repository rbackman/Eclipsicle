#pragma once
#include "shared.h"
#include "leds.h"
#include <FastLED.h>
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
class StripState
{
private:
    CRGB *leds;
    int stripIndex;
    int gravityPosition = 0;

    SliderParams sliderParams;
    float timeScale = 1;
    bool invert = false;
    bool centered = false;
    bool blackAndWhite = false;
    bool loopAnim = true;
    float multiplier = 1;
    int numLEDS = 128;
    bool cycle = false;
    RandomParams randomParams;
    int scrollSpeed = 1;
    int scrollPos = 0;
    float soundScale = 1;
    bool invertLEDs = false;
    LED_STATE ledState = LED_STATE_IDLE;
    Particle particles[10];
    Particle spawnType;
    int spawnRate = 2;
    bool accelMode = false;

public:
    StripState(CRGB row[], LED_STATE state, int numLEDS, int LED_PIN, int STRIP_INDEX, bool invert);
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
                return;
            }
        }
        if (!used)
        {
            Serial.println("No free particles");
        }
    }

    void spawnParticle()
    {
        spawnParticle(-spawnType.width, spawnType.velocity, spawnType.hueStart, spawnType.hueEnd, spawnType.brightness, spawnType.width, spawnType.life);
    }
    void updateRandomParticles();
    void updateParticles();
    void setScrollSpeed(int speed)
    {
        scrollSpeed = speed;
    }
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

    void respondToParameter(parameter_message parameter);
    bool respondToText(String command);

    void clearPixels();
    void clearPixel(int index);
    void setPixel(int index, led color);
    void setPixel(int index, int r, int g, int b);

    void toggleMode();
    void setScale(float scale)
    {
        if (scale < 1)
        {
            scale = 1;
        }
        soundScale = scale;
    }

    void update();
    String getStripState();
};