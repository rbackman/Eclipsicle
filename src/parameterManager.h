#pragma once
#include "shared.h"
class ParameterManager
{
public:
    ParameterManager(std::string name, std::vector<ParameterID> filterParams = {});

    int getValue(ParameterID paramID);
    float getFloat(ParameterID paramID);
    bool getBool(ParameterID paramID);
    void setValue(ParameterID paramID, int value);
    void setBool(ParameterID paramID, bool value);
    bool isBoolParameter(ParameterID id);

    IntParameter getIntParameter(ParameterID id);
    BoolParameter getBoolParameter(ParameterID id);
    const std::string getParameterName(ParameterID type)
    {
        for (auto it = intParams.begin(); it != intParams.end(); ++it)
        {
            if (it->id == type)
            {
                return it->name.c_str();
            }
        }

        for (auto it = boolParams.begin(); it != boolParams.end(); ++it)
        {
            if (it->id == type)
            {
                return it->name.c_str();
            }
        }
        return "UNKNOWN";
    }
    virtual void respondToParameterMessage(parameter_message parameter);
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
    std::string name;
};