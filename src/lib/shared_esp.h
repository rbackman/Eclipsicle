
#pragma once
#include "shared.h"
#include <Arduino.h>
#include "animations.h"
#include "stripState.h"
#include "ArduinoJson.h"

void printBytes(ByteRow data)
{
    Serial.println("bytes -->");
    for (int i = 0; i < data.size(); ++i)
    {
        Serial.printf("%d ", data[i]);
    }
    Serial.println("<-- as bytes");
}

std::string getAnimationInfoJsonForStrip(StripState *stripState)
{

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    root["type"] = "animations";
    JsonObject data = root["data"].to<JsonObject>();
    for (const auto &pair : ANIMATION_TYPE_NAMES)
    {
        ANIMATION_TYPE type = pair.first;
        const std::string &name = pair.second;
        std::unique_ptr<StripAnimation> anim;
        try
        {
            anim = makeAnimation(stripState, type, 0, 0, {});
        }
        catch (...)
        {
            continue;
        }
        JsonObject obj = data[name].to<JsonObject>();
        obj["id"] = (int)type;
        JsonArray arr = obj["params"].to<JsonArray>();
        if (anim)
        {
            for (const auto &p : anim->getIntParameters())
                arr.add(p.id);
            for (const auto &p : anim->getFloatParameters())
                arr.add(p.id);
            for (const auto &p : anim->getBoolParameters())
                arr.add(p.id);
        }
    }
    std::string out;
    serializeJson(doc, out);
    return out;
}
std::string getStripStateJson(StripState *stripState, bool verbose)
{
    JsonDocument doc;
    doc["type"] = "stripState";
    doc["strip"] = stripState->getStripIndex() + 1;
    doc["state"] = getLedStateName(stripState->getLedState());
    if (verbose)
    {
        JsonArray arr = doc["animations"].to<JsonArray>();
        for (const auto &anim : stripState->getAnimations())
        {
            JsonObject a = arr.add<JsonObject>();
            a["type"] = getAnimationName(anim->getAnimationType()).c_str();
            a["start"] = anim->getstart();
            a["end"] = anim->getend();
            JsonObject params = a["params"].to<JsonObject>();
            for (const auto &p : anim->getIntParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
            for (const auto &p : anim->getFloatParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
            for (const auto &p : anim->getBoolParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
        }
    }
    std::string output;
    serializeJson(doc, output);
    return output;
}
