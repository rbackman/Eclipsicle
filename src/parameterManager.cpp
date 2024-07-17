
#include "parameterManager.h"
#include "shared.h"
ParameterManager::ParameterManager(std::string name) : name(name)
{
    auto intParameterList = getDefaultParameters();
    auto boolParameterList = getDefaultBoolParameters();
    intParams.reserve(intParameterList.size());
    boolParams.reserve(boolParameterList.size());

    // set initial parameters. TODO: maybe save these to EEPROM
    for (int i = 0; i < intParameterList.size(); i++)
    {
        intParams.push_back({intParameterList[i].id, intParameterList[i].name, intParameterList[i].value, intParameterList[i].min, intParameterList[i].max, intParameterList[i].scale});
    }

    for (int i = 0; i < boolParameterList.size(); i++)
    {
        boolParams.push_back({boolParameterList[i].id, boolParameterList[i].name, boolParameterList[i].value});
    }
}

IntParameter ParameterManager::getIntParameter(ParameterID id)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (id == intParams[i].id)
        {
            return intParams[i];
        }
    }
    Serial.printf("Parameter not found %d %s\n", id, getParameterName(id));
    return IntParameter();
}
BoolParameter ParameterManager::getBoolParameter(ParameterID id)
{
    for (int i = 0; i < boolParams.size(); i++)
    {
        if (id == boolParams[i].id)
        {
            return boolParams[i];
        }
    }
    Serial.printf("Parameter not found %d %s\n", id, getParameterName(id));
    return BoolParameter();
}


void ParameterManager::setValue(ParameterID id, int val)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (intParams[i].id == id)
        {
            intParams[i].value = val;
            paramChanged = true;
            return;
        }
    }
    Serial.printf("Cant set Int Parameter, not found %d %s for %s\n", id, getParameterName(id), name.c_str());
}
void ParameterManager::setBool(ParameterID id, bool val)
{
    for (int i = 0; i < boolParams.size(); i++)
    {
        if (boolParams[i].id == id)
        {
            boolParams[i].value = val;
            paramChanged = true;
            return;
        }
    }
    Serial.printf("Cant set Bool Parameter, not found %d %s for %s\n", id, getParameterName(id), name.c_str());
}

// float getFloat(ParameterID paramID);
// bool getBool(ParameterID paramID);
// void setValue(ParameterID paramID, int value);
// void setBool(ParameterID paramID, bool value);

float ParameterManager::getFloat(ParameterID id)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (intParams[i].id == id)
        {
            return intParams[i].value * intParams[i].scale;
        }
    }
    Serial.printf("Float Parameter not found %d %s for %s\n", id, getParameterName(id), name.c_str());
    return 0;
}

int ParameterManager::getValue(ParameterID id)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (intParams[i].id == id)
        {
            if (intParams[i].scale == 1)
            { // normal int
                return intParams[i].value;
            }

            return (int)(intParams[i].value * intParams[i].scale);
        }
    }
    Serial.printf("Int Parameter not found %d %s for %s\n", id, getParameterName(id), name.c_str());
    return 0;
}
bool ParameterManager::getBool(ParameterID id)
{
    for (int i = 0; i < boolParams.size(); i++)
    {
        if (boolParams[i].id == id)
        {
            return boolParams[i].value;
        }
    }
    Serial.printf("Bool Parameter not found %d %s for %s\n", id, getParameterName(id), name.c_str());
    return false;
}

bool ParameterManager::isBoolParameter(ParameterID id)
{
    for (int i = 0; i < boolParams.size(); i++)
    {
        if (boolParams[i].id == id)
        {
            return true;
        }
    }
    return false;
}
void ParameterManager::respondToParameterMessage(parameter_message parameter)
{

    if (isBoolParameter(parameter.paramID))
    {

        setBool(parameter.paramID, parameter.boolValue);
        return;
    }

    setValue(parameter.paramID, parameter.value);
}