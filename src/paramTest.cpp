#ifdef PARAM_TEST
#include "parameterManager.h"
#include "shared.h"

ParameterManager *parameterManager;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Param Test");
    Serial.println("Setting Value");
    parameterManager = new ParameterManager("Test");
    parameterManager->setValue(PARAM_WIDTH, 10);
    Serial.println("Getting Value");
    Serial.println(parameterManager->getValue(PARAM_WIDTH));
}
void loop()
{
}

#endif