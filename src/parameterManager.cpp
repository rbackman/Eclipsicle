
#include "parameterManager.h"
#include "shared.h"

ParameterManager::ParameterManager(std::string name, std::vector<ParameterID> filterParams) : name(name) {
    auto intParameterList = getDefaultParameters();
    auto boolParameterList = getDefaultBoolParameters();
    intParams.reserve(intParameterList.size());
    boolParams.reserve(boolParameterList.size());

    // If filterParams is empty, copy all parameters
    if (filterParams.empty()) {
        filterParams.reserve(intParameterList.size() + boolParameterList.size());
        for (const auto& param : intParameterList) {
            filterParams.push_back(param.id);
        }
        for (const auto& param : boolParameterList) {
            filterParams.push_back(param.id);
        }
    }

    // Set initial parameters based on filterParams
    for (const auto& param : intParameterList) {
        if (std::find(filterParams.begin(), filterParams.end(), param.id) != filterParams.end()) {
            intParams.push_back(param);
        }
    }

    for (const auto& param : boolParameterList) {
        if (std::find(filterParams.begin(), filterParams.end(), param.id) != filterParams.end()) {
            boolParams.push_back(param);
        }
    }

    Serial.printf("Parameter Manager Initialized %s intParams:%d boolParams:%d \n", name.c_str(), intParams.size(), boolParams.size());
     
    for (const auto& iParam : intParams) {
        Serial.printf("Int Param %d %s %d\n", iParam.id, iParam.name.c_str(),iParam.value);
    }
    for (const auto& bParam : boolParams) {
        Serial.printf("Bool Param %d %s %s \n", bParam.id, bParam.name.c_str(),bParam.value ? "true" : "false");
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
    // Serial.printf("Cant set Int Parameter, not found %d %s for %s\n", id, getParameterName(id), name.c_str());
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
    // Serial.printf("Cant set Bool Parameter, not found %d %s for %s\n", id, getParameterName(id), name.c_str());
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