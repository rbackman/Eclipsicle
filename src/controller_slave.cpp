#ifdef CONTROLLER_SLAVE
#include <Arduino.h>

#include "./lib/pins.h"
#include "slave.h"

SlaveBoard *slaveBoard;

SerialManager *serialManager;
void setup()
{
  serialManager = new SerialManager(512);
  slaveBoard = new SlaveBoard(serialManager);

  slaveBoard->getLEDManager()->addStrip(0, 122, LED_STATE_MULTI_ANIMATION,
                                        {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                                        {{0, 54.5, -54.5, -49.5}, {49, 54.5, -54.5, 50}, {73, 26.5, -26.5, 21}, {94, 26.5, -26.5, -21}, {122, 54.5, -54.5, -49.5}});
  slaveBoard->getLEDManager()->addStrip(1, 122, LED_STATE_MULTI_ANIMATION,
                                        {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                                        {{0, 49.5, -54.5f, -54.5}, {49, -49.5, -54.5, -54.5}, {73, -21, -26.5, -26.5}, {94, 21, -26.5, -26.5}, {122, 49.5, -54.5f, -54.5}});
  slaveBoard->getLEDManager()->addStrip(2, 122, LED_STATE_MULTI_ANIMATION,
                                        {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
                                        {{0, -54.5, -54.5, -49.5}, {49, -54.5, -54.5, 49.5}, {73, -26.5, -26.5, 21}, {94, -26.5, -26.5, -21}, {122, -54.5, -54.5, -49.5}});
  slaveBoard->getLEDManager()->initStrips();

  Serial.println("Controller Slave initialized ");
}
void loop()
{

  slaveBoard->loop();
}

#endif