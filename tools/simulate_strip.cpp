#include "arduino_stub/Arduino.h"
#include "../src/shared.h"

String getLedStateName(LED_STATE state) {
    switch(state) {
        case LED_STATE_IDLE: return String("IDLE");
        case LED_STATE_SINGLE_ANIMATION: return String("SINGLEANIMATION");
        case LED_STATE_MULTI_ANIMATION: return String("MULTIANIMATION");
        case LED_STATE_POINT_CONTROL: return String("POINTCONTROL");
    }
    return String("UNKNOWN");
}

#include "../src/stripState.h"
#include "../src/animations.h"
#include <FastLED.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    setVerbose(false);
    StripState strip(LED_STATE_SINGLE_ANIMATION, 10, 0);
    strip.addAnimation(ANIMATION_TYPE_BRICKS, 0, 9, {{PARAM_WIDTH, 2}, {PARAM_VELOCITY, 1}});
    for (int step=0; step<20; ++step) {
        strip.update();
        for(int i=0;i<strip.getNumLEDS(); ++i) {
            auto c = strip.leds[i];
            std::cout << (c.r ? '#': '.');
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
