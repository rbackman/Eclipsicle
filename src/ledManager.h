#pragma once
#ifdef USE_LEDS
#include "stripState.h"

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
    LEDManager(std::string name);
    LEDManager(std::string name, std::vector<StripState *> strips);
    void initStrips();
    void setGravityPosition(float position);
    void update();
    void setLEDImage(image_message image);
    void setBrightness(int brightness);
    void setAll(led color);
    bool handleLEDCommand(String command);
    void setLED(int ledIndex, led color);
    void toggleMode();
    String getStripState(bool verbose = false);
    int getCurrentStrip();
    std::vector<StripState*> &getStrips() { return stripStates; }
    bool respondToParameterMessage(parameter_message parameter);
};

#endif