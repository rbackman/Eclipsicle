#pragma once
#ifdef USE_LEDS
#include "stripState.h"
#include <string>

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
    std::string getStripState(bool verbose = false);
    std::string getStripStateJson(bool verbose = false);
    std::string getStripStateCompact(bool verbose = false);
    std::string getAnimationInfoJson();
    int getCurrentStrip();
    std::vector<StripState *> &getStrips() { return stripStates; }
    bool respondToParameterMessage(parameter_message parameter);
};

#endif