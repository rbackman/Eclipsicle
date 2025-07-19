// parameterManager.h
#pragma once
#include "shared.h"
#include <ArduinoJson.h>

#include <vector>
typedef bool (*ParameterChangeListener)(parameter_message parameter);

// Stores int, float and bool parameters for a component.
// Parameter IDs are defined in shared.h so firmware and UI share the same enum.

static void confirmParameters()
{
    Serial.println("Confirming parameters...");

    JsonDocument paramJson; // adjust size as needed
    paramJson["type"] = "parameters";
    JsonObject data = paramJson["data"].to<JsonObject>();

    auto paramList = getParameterNames();

    for (const auto &bParam : getDefaultBoolParameters())
    {
        const char *name = paramList[bParam.id].c_str();
        JsonObject obj = data[name].to<JsonObject>();
        obj["id"] = bParam.id;
        obj["type"] = "bool";
        obj["value"] = bParam.value;
        obj["name"] = bParam.name;
    }

    for (const auto &iParam : getDefaultIntParameters())
    {
        const char *name = paramList[iParam.id].c_str();
        JsonObject obj = data[name].to<JsonObject>();
        obj["id"] = iParam.id;
        obj["type"] = "int";
        obj["value"] = iParam.value;
        obj["name"] = iParam.name;
        obj["min"] = iParam.min;
        obj["max"] = iParam.max;
    }

    for (const auto &fParam : getDefaultFloatParameters())
    {
        const char *name = paramList[fParam.id].c_str();
        JsonObject obj = data[name].to<JsonObject>();
        obj["id"] = fParam.id;
        obj["type"] = "float";
        obj["value"] = fParam.value;
        obj["name"] = fParam.name;
        obj["min"] = fParam.min;
        obj["max"] = fParam.max;
    }

    serializeJson(paramJson, Serial);
    Serial.println(";");
}

class ParameterManager
{
    // parameter change listeners
    std::vector<ParameterChangeListener> listeners;

public:
    ParameterManager(std::string name, std::vector<ParameterID> filterParams = {}, std::map<ParameterID, float> paramOverrides = {});
    void addParameterChangeListener(ParameterChangeListener listener)
    {
        listeners.push_back(listener);
    }
    bool hasParameter(ParameterID paramID);
    int getInt(ParameterID paramID);
    float getFloat(ParameterID paramID);
    bool getBool(ParameterID paramID);
    void setInt(ParameterID paramID, int value);
    void setBool(ParameterID paramID, bool value);
    void setFloat(ParameterID paramID, float value);
    void setParameters(std::map<ParameterID, float> params)
    {

        for (const auto &param : params)
        {
            if (isFloatParameter(param.first))
            {

                setFloat(param.first, param.second);
            }
            else if (isIntParameter(param.first))
            {

                setInt(param.first, (int)param.second);
            }
            else if (isBoolParameter(param.first))
            {

                setBool(param.first, (bool)param.second);
            }
        }
    }
    IntParameter getIntParameter(ParameterID id);
    BoolParameter getBoolParameter(ParameterID id);
    FloatParameter getFloatParameter(ParameterID id);

    virtual bool handleParameterMessage(parameter_message parameter);
    bool handleJsonMessage(JsonDocument &doc);
    bool handleTextMessage(std::string message);
    bool parameterChanged()
    {
        if (paramChanged)
        {
            paramChanged = false;
            return true;
        }
        return false;
    }
    const std::string getName()
    {
        return name;
    }

    const std::vector<IntParameter> &getIntParameters() const { return intParams; }
    const std::vector<BoolParameter> &getBoolParameters() const { return boolParams; }
    const std::vector<FloatParameter> &getFloatParameters() const { return floatParams; }
    ParameterID getParameterID(std::string name)
    {
        auto pnames = getParameterNames();
        for (int i = 0; i < pnames.size(); i++)
        {
            if (pnames[i] == name)
            {
                return (ParameterID)i;
            }
        }
        return PARAM_UNKNOWN;
    }

private:
    bool paramChanged = true;
    std::vector<BoolParameter> boolParams = {};
    std::vector<IntParameter> intParams = {};
    std::vector<FloatParameter> floatParams = {};
    std::string name;
    // std::vector<ParameterChangeListener> listeners = {};
};