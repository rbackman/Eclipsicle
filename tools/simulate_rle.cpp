#include "arduino_stub/Arduino.h"
#include "../src/lib/stripState.h"
#include "../src/lib/animations.h"
#include "../src/lib/shared.h"
#include <FastLED.h>

std::string getLedStateName(LED_STATE state) {
    switch(state) {
        case LED_STATE_IDLE: return "IDLE";
        case LED_STATE_SINGLE_ANIMATION: return "SINGLEANIMATION";
        case LED_STATE_MULTI_ANIMATION: return "MULTIANIMATION";
        case LED_STATE_POINT_CONTROL: return "POINTCONTROL";
    }
    return "UNKNOWN";
}

int main() {
    setVerbose(false);
    StripState strip(LED_STATE_SINGLE_ANIMATION, 30, 0);
    strip.setSimulate(1); // print RLE each update
    strip.addAnimation(ANIMATION_TYPE_RAINBOW, 0, 29, {});
    for(int i=0;i<100;++i) {
        strip.update();
        delay(50);
    }
    return 0;
}
