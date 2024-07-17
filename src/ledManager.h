#pragma once

#include "stripState.h"


class LEDManager 
{

private:
    int currentStrip = 0;
    bool safeLight = false; // the last led in the strip points back so it can be seen
    LedMatrix *ledMatrix;
    float gravityPosition = 0;
    // strip state array
    StripState* stripStates[NUM_STRIPS];

    int lastUpdate = 0;
    int fps = 60;
    ParameterManager *parameterManager;
public:
    LEDManager( ParameterManager *parameterManager);
    void setCurrentStrip(int strip);
    void setGravityPosition(float position);
    void update();
    void setLEDImage(image_message image);
    void setBrightness(int brightness);
    void setAll(led color);
    bool handleLEDCommand(String command);
 

 
    void toggleMode();
    String getStripState();
};
