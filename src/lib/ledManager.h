#pragma once
#ifdef USE_LEDS
#include "stripState.h"
#include <string>

class LEDManager : public ParameterManager
{

private:
    LedMatrix *ledMatrix;

    std::vector<StripState *> stripStates;
    int lastUpdate = 0;
    int fps = 60;
    int getLEDPin(int stripIndex) const;

public:
    LEDManager();
    void setAnimation(std::string animationName);
    void addStrip(int stripIndex, int numLEDS, LED_STATE state, std::vector<AnimationParams> animations, std::vector<Node3D> nodes)
    {

        StripState *stripState = new StripState(state, numLEDS, stripIndex, nodes);
        if (isVerbose())
        {
            Serial.printf("Adding LED strip with pin %d and %d LEDs  \n", getLEDPin(stripIndex), numLEDS);
        }
        for (int j = 0; j < animations.size(); j++)
        {
            AnimationParams anim = animations[j];
            if (isVerbose())
            {
                Serial.printf("Adding animation %s to strip %d\n", getAnimationName(anim.type), stripState->getStripIndex());
            }
            stripState->addAnimation(anim.type, anim.start, anim.end, anim.params);
        }

        stripStates.push_back(stripState);
    }
    void initStrips();
    void setGravityPosition(float position);
    void update();
    void setLEDImage(image_message image);
    void setBrightness(int brightness);
    void setAll(led color);

    void setLED(int ledIndex, led color);
    void toggleMode();
    std::string getStripState(bool verbose = false);
    std::string getStripsStateJson(bool verbose = false);
    std::string getStripStateCompact(bool verbose = false);
    std::string getAnimationInfoJson();
    int getCurrentStrip();
    std::vector<StripState *> &getStrips() { return stripStates; }
    bool handleParameterMessage(parameter_message parameter);
    bool handleString(String command);
};

#endif