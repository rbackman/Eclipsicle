#pragma once
#include "shared.h"
#include "leds.h"
#include <FastLED.h>
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
class StripState  
{
private:
    CRGB *leds;
    int stripIndex;
    int gravityPosition = 0;

 bool invertLEDs = false;

    int numLEDS = 128;

    float scrollPos = 0;

    LED_STATE ledState = LED_STATE_IDLE;
    Particle particles[10];
  
    ParameterManager* parameterManager;
public:
    StripState(ParameterManager* parameterManager,CRGB row[], LED_STATE state, int numLEDS, int LED_PIN, int STRIP_INDEX, bool invert);
   
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
            int width = parameterManager->getValue(PARAM_PARTICLE_WIDTH);
            int velocity = parameterManager->getValue(PARAM_VELOCITY);
            int hueStart = parameterManager->getValue(PARAM_HUE);
            int hueEnd = parameterManager->getValue(PARAM_HUE_END);
            int brightness = parameterManager->getValue(PARAM_BRIGHTNESS);
            int life = parameterManager->getValue(PARAM_PARTICLE_LIFE);

            spawnParticle(-width, velocity, hueStart, hueEnd, brightness, width, life);
    };
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

   
    bool respondToText(String command);

    void clearPixels();
    void clearPixel(int index);
    void setPixel(int index, led color);
    void setPixel(int index, int r, int g, int b);

    void toggleMode();
   

    void update();
    String getStripState();
};