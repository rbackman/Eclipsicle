
#ifdef LED_MASTER
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

  delay(1000);
  serialManager = new SerialManager(1024);
  slaveBoard = new SlaveBoard(serialManager);

  //  ANIMATION_TYPE type;
  //   int start = 0;
  //   int end = -1; // -1 means all LEDs

  //   std::map<ParameterID, float> params = {};

  AnimationParams params = {ANIMATION_TYPE_PARTICLES, -1, -1, {{PARAM_HUE, 100}, {PARAM_HUE_END, 300}, {PARAM_TIME_SCALE, 50}, {PARAM_SPAWN_RATE, 80}}};

  std::vector<Node3D> nodes = {
      {0, 54.5, -54.5, -49.5},
      {10, 54.5, -54.5, 50},

  };
#ifdef USE_LEDS
  slaveBoard->getLEDManager()->addStrip(3, 32, LED_STATE_MULTI_ANIMATION,
                                        {params},
                                        nodes);

  slaveBoard->getLEDManager()->initStrips();
#endif

  Serial.println("LED Master initialized ");
}
void loop()
{

  slaveBoard->loop();
}

#endif