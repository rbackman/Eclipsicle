//   animations.h
#pragma once
#include "shared.h"
#include "stripState.h"

struct Particle
{
    bool active = false;
    int life = -1;
    float position = 0;
    float maxSpeed = 1.5;
    float velocity = 0.2;
    float acceleration = 0.5;

    float fade = 0.9;
    int randomDrift = 5;

    int hueStart;
    int hueEnd;
    int brightness;
    int width;
};

// Forward declaration to fix 'StripState' does not name a type error
class StripState;

struct SliderParams
{
    int position;
    int width;
    int hueshift;
    int repeat;
    bool useGravity;
};

class StripAnimation
{
    ANIMATION_TYPE animationType;

    std::map<int, float> params;

protected:
    StripState *stripState;
    int scrollPos = 0;

public:
    void update(StripState *strip) = 0;
    StripAnimation(StripState *state, ANIMATION_TYPE type, std::map<int, float> params) : stripState(state), animationType(type), params(params) {}
    void setParam(int id, float value)
    {
        params[id] = value;
    }
    float getParam(int id, float defaultValue = 0)
    {
        if (params.find(id) != params.end())
        {
            return params[id];
        }
        return defaultValue;
    }

    void respondToParameterMessage(parameter_message parameter)
    {
        if (params.find(parameter.paramID) != params.end())
        {
            params[parameter.paramID] = parameter.value;
        }
    }
};

class ParticleAnimation : public StripAnimation
{
    Particle particles[10];
    bool randomMode = false;
    void fadeParticleTail(int position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction);

    void spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life);
    void spawnParticle();
    void updateRandomParticles();
    void updateParticles();

public:
    void update(StripState *strip);
    ParticleAnimation(StripState *state, bool random) : StripAnimation(state, ANIMATION_TYPE_PARTICLES, {
                                                                                                            {PARAM_HUE, 100},
                                                                                                            {PARAM_HUE_END, 230},
                                                                                                            {PARAM_VELOCITY, 12},
                                                                                                            {PARAM_BRIGHTNESS, 50},
                                                                                                            {PARAM_PARTICLE_WIDTH, 5},
                                                                                                            {PARAM_PARTICLE_LIFE, 20},
                                                                                                            {PARAM_RANDOM_DRIFT, 0},
                                                                                                            {PARAM_ACCELERATION, 0},
                                                                                                            {PARAM_MAX_SPEED, 20},
                                                                                                        })
    {
        randomMode = random;
    }
};

class RainbowAnimation : public StripAnimation
{
public:
    void update(StripState *strip);
    RainbowAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_RAINBOW, {
                                                                                            {PARAM_SCROLL_SPEED, 0.5},
                                                                                            {PARAM_RAINBOW_REPEAT, 1},
                                                                                            {PARAM_RAINBOW_OFFSET, 0},
                                                                                            {PARAM_BRIGHTNESS, 255},
                                                                                        })
    {
    }
};

class RandomAnimation : public StripAnimation
{
public:
    void update(StripState *strip);
    RandomAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_RANDOM, {
                                                                                          {PARAM_SCROLL_SPEED, 1.0},
                                                                                          {PARAM_RANDOM_OFF, 0},
                                                                                          {PARAM_BRIGHTNESS, 255},
                                                                                      })
    {
    }
};

class SliderAnimation : public StripAnimation
{
    SliderParams params;

public:
    void update(StripState *strip);
    SliderAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_SLIDER, {
                                                                                          {PARAM_SLIDER_POSITION, 0},
                                                                                          {PARAM_SLIDER_WIDTH, 10},

                                                                                          {PARAM_SLIDER_REPEAT, 1},

                                                                                      })
    {
    }
};