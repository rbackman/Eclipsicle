#ifdef LIGHT_SWORD

#include "shared.h"

#ifdef USE_SENSORS
SensorGrid grid = {
    SensorState(SLIDER, 34, SLIDER1),

    SensorState(BUTTON, 21, BUTTON_UP),
    SensorState(BUTTON, 25, BUTTON_DOWN),

};
sensorManager = new SensorManager(grid);
#endif

makeRig("Lightsword", {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC});
addStripToRig("Lightsword", 3, 128, LED_STATE_MULTI_ANIMATION,
              {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
              {{0, 0.0f, -54.5f, -54.5f}, {127, 0.0f, 54.5f, 54.5f}});
#endif