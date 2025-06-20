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

class StripAnimation : public ParameterManager
{
    ANIMATION_TYPE animationType;

protected:
    StripState *stripState;
    int scrollPos = 0;

public:
    virtual void update(StripState *strip) = 0;
    StripAnimation(StripState *state, ANIMATION_TYPE type, std::vector<ParameterID> params) : ParameterManager(getAnimationName(type), params)
    {
        this->stripState = state;
        this->animationType = type;
    }
    ANIMATION_TYPE getAnimationType()
    {
        return animationType;
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

    ParticleAnimation(StripState *state, bool random) : StripAnimation(state, ANIMATION_TYPE_PARTICLES, {PARAM_HUE, PARAM_HUE_END, PARAM_VELOCITY, PARAM_BRIGHTNESS, PARAM_PARTICLE_WIDTH, PARAM_PARTICLE_LIFE, PARAM_PARTICLE_FADE, PARAM_RANDOM_DRIFT, PARAM_ACCELERATION, PARAM_MAX_SPEED, PARAM_SPAWN_RATE, PARAM_TIME_SCALE, PARAM_CYCLE})
    {
        this->stripState = state;
        this->randomMode = random;
    }
};

class RainbowAnimation : public StripAnimation
{
public:
    void update(StripState *strip);

    RainbowAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_RAINBOW, {PARAM_SCROLL_SPEED, PARAM_RAINBOW_REPEAT, PARAM_RAINBOW_OFFSET, PARAM_BRIGHTNESS})
    {
    }
};

class RandomAnimation : public StripAnimation
{
public:
    void update(StripState *strip);
    RandomAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_RANDOM, {PARAM_SCROLL_SPEED, PARAM_RANDOM_OFF, PARAM_BRIGHTNESS})
    {
    }
};

class SliderAnimation : public StripAnimation
{

public:
    void update(StripState *strip);
    SliderAnimation(StripState *state) : StripAnimation(state, ANIMATION_TYPE_SLIDER, {PARAM_SLIDER_POSITION, PARAM_SLIDER_WIDTH, PARAM_SLIDER_REPEAT})
    {
    }
};