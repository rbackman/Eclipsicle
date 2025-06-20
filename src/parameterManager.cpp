// parameterManager.cpp
#include "parameterManager.h"
#include "shared.h"

ParameterManager::ParameterManager(std::string name, std::vector<ParameterID> filterParams) : name(name)
{
    auto intParameterList = getDefaultIntParameters();
    auto boolParameterList = getDefaultBoolParameters();
    auto floatParameterList = getDefaultFloatParameters();
    intParams.reserve(intParameterList.size());
    boolParams.reserve(boolParameterList.size());
    floatParams.reserve(floatParameterList.size());

    if (filterParams.empty())
    {

        Serial.printf("No filter params provided for %s\n", name.c_str());
        return;
    }

    // Set initial parameters based on filterParams
    for (const auto &param : intParameterList)
    {
        if (std::find(filterParams.begin(), filterParams.end(), param.id) != filterParams.end())
        {
            intParams.push_back(param);
        }
    }

    for (const auto &param : boolParameterList)
    {
        if (std::find(filterParams.begin(), filterParams.end(), param.id) != filterParams.end())
        {
            boolParams.push_back(param);
        }
    }
    for (const auto &param : floatParameterList)
    {
        if (std::find(filterParams.begin(), filterParams.end(), param.id) != filterParams.end())
        {
            floatParams.push_back(param);
        }
    }

    if (isVerbose())
    {
        Serial.printf("Parameter Manager Initialized %s intParams:%d boolParams:%d floatParams:%d\n", name.c_str(), intParams.size(), boolParams.size(), floatParams.size());

        for (const auto &iParam : intParams)
        {
            Serial.printf("Int Param %d %s %d\n", iParam.id, iParam.name.c_str(), iParam.value);
        }
        for (const auto &bParam : boolParams)
        {
            Serial.printf("Bool Param %d %s %s \n", bParam.id, bParam.name.c_str(), bParam.value ? "true" : "false");
        }
        for (const auto &fParam : floatParams)
        {
            Serial.printf("Float Param %d %s %f\n", fParam.id, fParam.name.c_str(), fParam.value);
        }
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
    IntParameter iParam = {id, getParameterName(id), 0, 0, 0};
    intParams.push_back(iParam);
    auto name = getParameterName(id);
    Serial.printf("Error: Int Parameter not found %d %s\n", id, name.c_str());
    return intParams.back();
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
    BoolParameter bParam = {id, getParameterName(id), false};
    boolParams.push_back(bParam);

    auto name = getParameterName(id);
    Serial.printf("Error: Parameter not found %d %s\n", id, name.c_str());

    return boolParams.back();
}

FloatParameter ParameterManager::getFloatParameter(ParameterID id)
{
    for (int i = 0; i < floatParams.size(); i++)
    {
        if (id == floatParams[i].id)
        {
            return floatParams[i];
        }
    }
    FloatParameter fParam = {id, getParameterName(id), 0.0, 0.0, 1.0};
    floatParams.push_back(fParam);
    auto name = getParameterName(id);
    Serial.printf("Error: Parameter not found %d %s\n", id, name.c_str());
    return floatParams.back();
}
void ParameterManager::setInt(ParameterID id, int val)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (intParams[i].id == id)
        {
            intParams[i].value = val;
            paramChanged = true;
            if (isVerbose())
                Serial.printf("updating parameter %d %s for %s\n", id, getParameterName(id).c_str(), name.c_str());
            return;
        }
    }
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
void ParameterManager::setFloat(ParameterID id, float val)
{
    for (int i = 0; i < floatParams.size(); i++)
    {
        if (floatParams[i].id == id)
        {
            floatParams[i].value = val;
            paramChanged = true;
            return;
        }
    }
    // Serial.printf("Cant set Float Parameter, not found %d %s for %s\n", id, getParameterName(id), name.c_str());
}

float ParameterManager::getFloat(ParameterID id)
{
    for (int i = 0; i < floatParams.size(); i++)
    {
        if (floatParams[i].id == id)
        {
            return floatParams[i].value;
        }
    }
    auto pname = getParameterName(id);
    floatParams.push_back({id, pname, 0.0, 0.0, 1.0});

    Serial.printf("Error Float Parameter not found %d %s for %s\n", id, pname.c_str(), name.c_str());
    return 0;
}

int ParameterManager::getInt(ParameterID id)
{
    for (int i = 0; i < intParams.size(); i++)
    {
        if (intParams[i].id == id)
        {

            return intParams[i].value;
        }
    }
    auto pname = getParameterName(id);
    intParams.push_back({id, pname, 0, 0, 0});
    Serial.printf("Error Int Parameter not found %d %s for %s\n", id, pname.c_str(), name.c_str());
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
    auto pname = getParameterName(id);
    boolParams.push_back({id, pname, false});

    Serial.printf("Error Bool Parameter not found %d %s for %s\n", id, pname.c_str(), name.c_str());
    return false;
}

bool ParameterManager::respondToParameterMessage(parameter_message parameter)
{
    Serial.printf("Responding to parameter message %d %d %d\n", parameter.type, parameter.paramID, parameter.value);

    if (!listeners.empty())
    {
        Serial.printf("%s ParameterManager sending parameters %d %d %d\n", name, parameter.type, parameter.paramID, parameter.value);
    }
    for (int i = 0; i < listeners.size(); i++)
    {
        auto listener = listeners[i];

        if (listener)
        {
            if (listener(parameter))
            {
                Serial.printf("Listener %d handled parameter message %d %d %d\n", i, parameter.type, parameter.paramID, parameter.value);
                return true;
            }
        }
    }
    if (isBoolParameter(parameter.paramID))
    {

        setBool(parameter.paramID, parameter.boolValue);
        return true;
    }
    else if (isFloatParameter(parameter.paramID))
    {
        setFloat(parameter.paramID, parameter.value);
        return true;
    }
    else if (isIntParameter(parameter.paramID))
    {
        setInt(parameter.paramID, parameter.value);
        return true;
    }

    Serial.printf("Error: Parameter not found %d %s for %s\n", parameter.paramID, getParameterName(parameter.paramID).c_str(), name.c_str());
    return false;
}
bool ParameterManager::handleJsonMessage(JsonDocument &doc)
{
    try
    {
        bool handled = false;
        if (doc["param"].is<std::string>())

        {

            parameter_message parameter;
            parameter.type = MESSAGE_TYPE_PARAMETER;
            std::string paramName = doc["param"].as<std::string>();
            if (paramName.empty())
            {
                Serial.println("Parameter name is null in JSON message");
                return false;
            }
            parameter.paramID = getParameterID(paramName);
            if (parameter.paramID == PARAM_UNKNOWN)
            {
                Serial.printf("Unknown parameter name in JSON message: %s\n", paramName);
                return false;
            }

            if (isIntParameter(parameter.paramID))
            {
                if (doc["value"].is<int>())
                {
                    if (isVerbose())

                        Serial.printf("Set parameter %s to %d for %s\n", paramName, parameter.value, name.c_str());
                    parameter.value = doc["value"].as<int>();
                    handled = true;
                }
                else
                {
                    Serial.printf("Parameter %s  not sent as int\n", paramName);
                    return false;
                }
                // Serial.printf("Set parameter %s to %d\n", paramName, parameter.value);
            }
            else if (isFloatParameter(parameter.paramID))
            {
                if (doc["value"].is<float>())
                {
                    handled = true;
                    parameter.value = (int)doc["value"].as<float>();
                    if (isVerbose())
                    {
                        Serial.printf("Set parameter %s to %f for %s\n", paramName, parameter.value, name.c_str());
                    }
                }
                else
                {
                    Serial.printf("Parameter %s  not sent as float\n", paramName);
                    return false;
                }
            }
            else if (isBoolParameter(parameter.paramID))
            {
                if (doc["value"].is<bool>())
                {
                    parameter.boolValue = doc["value"].as<bool>();
                    handled = true;
                    if (isVerbose())
                    {
                        Serial.printf("Set parameter %s to %s for %s\n", paramName, parameter.boolValue ? "true" : "false", name.c_str());
                    }
                }
                else
                {
                    Serial.printf("Parameter %s  not sent as bool\n", paramName);
                    return false;
                }
            }
            else
            {
            }
            if (!handled)
            {
                // Serial.println("No valid value found in parameter JSON message");
                if (isVerbose())
                {
                    Serial.printf("object %s does not declare Parameter %s   \n", name.c_str(), getParameterName(parameter.paramID).c_str());
                }
                return false;
            }
            if (isVerbose())
            {
                Serial.printf("set object %s parameter %s to ", name.c_str(), getParameterName(parameter.paramID).c_str());
                if (isIntParameter(parameter.paramID))
                {
                    Serial.printf("int param %d\n", parameter.value);
                }
                else if (isFloatParameter(parameter.paramID))
                {
                    Serial.printf("float param %f\n", parameter.value);
                }
                else if (isBoolParameter(parameter.paramID))
                {
                    Serial.printf("bool param %s\n", parameter.boolValue ? "true" : "false");
                }
            }
            respondToParameterMessage(parameter);
            return true;
        }
        else
        {
            Serial.println("Non parameter JSON message received  ");

            serializeJson(doc, Serial);
            Serial.println();
        }
    }
    catch (...)
    {
        Serial.println("Param Manager Error handling JSON message");
    }
    return false;
}

bool ParameterManager::handleTextMessage(std::string message)
{

    if (message.find("PARAM_") == 0)
    {
        int paramID = message.find(":");
        if (paramID != std::string::npos)
        {
            std::string param = message.substr(0, paramID);
            std::string value = message.substr(paramID + 1);

            try
            {

                int val = std::stoi(value);
                auto id = getParameterID(param);
                Serial.printf("Parameter Message: %s  value: %s  id: %d\n", param.c_str(), value.c_str(), id);
                setInt((ParameterID)id, val);
            }
            catch (...)
            {
                Serial.printf("Error parsing parameter message: %s", message);
            }
            return true;
        }
    }
    return false;
}