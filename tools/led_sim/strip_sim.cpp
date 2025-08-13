#include "stripState.h"
#include "animations.h"

#include <string>

std::string getLedStateName(LED_STATE state)
{
    switch (state)
    {
    case LED_STATE_IDLE:
        return "IDLE";
    case LED_STATE_SINGLE_ANIMATION:
        return "SINGLEANIMATION";
    case LED_STATE_MULTI_ANIMATION:
        return "MULTIANIMATION";
    case LED_STATE_POINT_CONTROL:
        return "POINTCONTROL";
    }
    return "UNKNOWN";
}

extern "C" {

StripState *stripsim_create(int led_count)
{
    return new StripState(LED_STATE_SINGLE_ANIMATION, led_count, 0);
}

void stripsim_destroy(StripState *s)
{
    delete s;
}

void stripsim_set_animation(StripState *s, int anim_type)
{
    if (!s)
        return;
    s->setAnimation(static_cast<ANIMATION_TYPE>(anim_type));
}

void stripsim_update(StripState *s)
{
    if (s)
        s->update();
}

const char *stripsim_get_rle(StripState *s)
{
    static std::string rle;
    if (!s)
        return "";
    rle = s->getCompressedLEDs();
    return rle.c_str();
}

bool stripsim_command(StripState *s, const char *cmd)
{
    if (!s || !cmd)
        return false;
    return s->handleTextMessage(std::string(cmd));
}

bool stripsim_parameter(StripState *s, const parameter_message *param)
{
    if (!s || !param)
        return false;
    return s->handleParameterMessage(*param);
}

}

