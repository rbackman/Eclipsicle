// parameterManager.h
#pragma once
#include "shared.h"
#include <ArduinoJson.h>

#include <vector>
typedef bool (*ParameterChangeListener)(parameter_message parameter);

// Stores int, float and bool parameters for a component.
// Parameter IDs are defined in shared.h so firmware and UI share the same enum.

class ParameterManager
{
    // parameter change listeners
    std::vector<ParameterChangeListener> listeners;

public:
    ParameterManager(std::string name, std::vector<ParameterID> filterParams = {});
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

    virtual bool respondToParameterMessage(parameter_message parameter);
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