#ifdef TESS_CONTROLLER

#include "master.h"
MasterBoard *master;

void setup()
{

  master = new MasterBoard();
  delay(1000); // Allow time for serial to initialize
}

void loop()
{
  master->update();
}

#endif