#include "leds.h"
std::string getLedStateName(LED_STATE state) {
    switch(state) {
        case LED_STATE_IDLE: return "IDLE";
        case LED_STATE_SINGLE_ANIMATION: return "SINGLEANIMATION";
        case LED_STATE_MULTI_ANIMATION: return "MULTIANIMATION";
        case LED_STATE_POINT_CONTROL: return "POINTCONTROL";
    }
    return "UNKNOWN";
}
