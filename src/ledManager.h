#pragma once
#ifdef USE_LEDS
#include "stripState.h"

struct LEDParams
{
    int ledPin;
    int numLEDS;
    int stripIndex;
    LED_STATE startState;
    bool reverse;
};

class LEDManager : public ParameterManager
{

private:
    bool safeLight = false; // the last led in the strip points back so it can be seen
    LedMatrix *ledMatrix;
    float gravityPosition = 0;
    // strip state array

    std::vector<StripState *> stripStates;
    int lastUpdate = 0;
    int fps = 60;

public:
    LEDManager(std::vector<LEDParams> strips);

    void setGravityPosition(float position);
    void update();
    void setLEDImage(image_message image);
    void setBrightness(int brightness);
    void setAll(led color);
    bool handleLEDCommand(String command);

    void toggleMode();
    String getStripState();
    int getCurrentStrip();
    void respondToParameterMessage(parameter_message parameter);
};

#endif