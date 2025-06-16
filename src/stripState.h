#pragma once

#ifdef USE_LEDS
#include "FastLED.h"
#include "shared.h"
#include "leds.h"

#include "parameterManager.h"
#include "animations.h"
#include <memory>

class StripAnimation;
class StripState : public ParameterManager
{

private:
    int stripIndex;
    int currentAnimation = 0;
    int gravityPosition = 0;
    int numLEDS = 128;
    bool invertLEDs = false;
    float beatSize = 0;

    float scrollPos = 0;

    std::vector<std::unique_ptr<StripAnimation>> animations;
    LED_STATE ledState = LED_STATE_IDLE;

public:
    bool isActive = true;
    CRGB *leds;
    StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX, bool invert);

    void setNumLEDS(int num)
    {
        numLEDS = num;
    }
    void addAnimation(ANIMATION_TYPE animis);
    void setAnimation(ANIMATION_TYPE animType)
    {
        ledState = LED_STATE_SINGLE_ANIMATION;
        animations.clear();
        addAnimation(animType);
    }
    int getNumLEDS()
    {
        return numLEDS;
    }
    void setGravityPosition(float position)
    {
        gravityPosition = (int)(position * numLEDS);
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