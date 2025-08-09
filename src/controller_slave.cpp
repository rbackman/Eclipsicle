#ifdef CONTROLLER_SLAVE
#include <Arduino.h>

#include "./lib/pins.h"
#include "slave.h"

SlaveBoard *slaveBoard;

SerialManager *serialManager;
#ifdef MESH_NET
MeshnetManager *meshManager;
#endif

void setup()
{
  serialManager = new SerialManager(1024);
  slaveBoard = new SlaveBoard(serialManager);

  //  ANIMATION_TYPE type;
  //   int start = 0;
  //   int end = -1; // -1 means all LEDs

  //   std::map<ParameterID, float> params = {};

  AnimationParams params = {ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}, {PARAM_SPAWN_RATE, 80}}};

  std::vector<Node3D> nodes = {
      {0, 54.5, -54.5, -49.5},
      {44, 54.5, -54.5, 50},
      {131, 26.5, -26.5, 21},
      {176, 26.5, -26.5, -21},
      {209, 54.5, -54.5, -49.5},
      {210, 0, 0, 0}, // back start
      {301, 0, 0, 0}, // back end
      {302, 0, 0, 0}, // front start
      {334, 0, 0, 0}  // front end
  };

  slaveBoard->getLEDManager()->addStrip(0, 337, LED_STATE_MULTI_ANIMATION,
                                        {params},
                                        nodes);
  slaveBoard->getLEDManager()->addStrip(1, 337, LED_STATE_MULTI_ANIMATION,
                                        {params},
                                        nodes);
  slaveBoard->getLEDManager()->addStrip(2, 337, LED_STATE_MULTI_ANIMATION,
                                        {params},
                                        nodes);
  slaveBoard->getLEDManager()->addStrip(3, 337, LED_STATE_MULTI_ANIMATION,
                                        {params},
                                        nodes);
  // slaveBoard->getLEDManager()->addStrip(1, 122, LED_STATE_MULTI_ANIMATION,
  //                                       {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
  //                                       {{0, 49.5, -54.5f, -54.5}, {49, -49.5, -54.5, -54.5}, {73, -21, -26.5, -26.5}, {94, 21, -26.5, -26.5}, {122, 49.5, -54.5f, -54.5}});
  // slaveBoard->getLEDManager()->addStrip(2, 122, LED_STATE_MULTI_ANIMATION,
  //                                       {{ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}}}},
  //                                       {{0, -54.5, -54.5, -49.5}, {49, -54.5, -54.5, 49.5}, {73, -26.5, -26.5, 21}, {94, -26.5, -26.5, -21}, {122, -54.5, -54.5, -49.5}});
  slaveBoard->getLEDManager()->initStrips();

  Serial.println("Controller Slave initialized ");
}
void loop()
{

  slaveBoard->loop();
}

#endif