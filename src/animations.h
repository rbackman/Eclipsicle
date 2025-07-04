//   animations.h
#pragma once
#include "shared.h"
#include "parameterManager.h"

class StripState;
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
    StripState *stripState;

protected:
    float scrollPos = 0.0f;
    int startLED = -1;
    int endLED = -1;

public:
    virtual void update() = 0;
    StripAnimation(StripState *state, int startLED, int endLED, ANIMATION_TYPE type, std::vector<ParameterID> params) : ParameterManager(getAnimationName(type), params)
    {
        this->stripState = state;
        this->animationType = type;
        this->startLED = startLED;
        this->endLED = endLED;
    }
    ANIMATION_TYPE getAnimationType()
    {
        return animationType;
    }
    int numLEDs()
    {
        return this->endLED - this->startLED + 1;
    }
    int getStartLED()
    {
        return startLED;
    }
    int getEndLED()
    {
        return endLED;
    }
    void setPixel(int index, led color);
    String describe();
};

#define NUM_PARTICLES 20
class ParticleAnimation : public StripAnimation
{
    Particle particles[NUM_PARTICLES];
    bool randomMode = false;

    void fadeParticleTail(float position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction);

    void spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life);
    void spawnParticle();
    void updateRandomParticles();
    void updateParticles();

public:
    void update();

    ParticleAnimation(StripState *state, bool random, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_PARTICLES, {PARAM_HUE, PARAM_HUE_END, PARAM_VELOCITY, PARAM_BRIGHTNESS, PARAM_PARTICLE_WIDTH, PARAM_PARTICLE_LIFE, PARAM_PARTICLE_FADE, PARAM_RANDOM_DRIFT, PARAM_ACCELERATION, PARAM_MAX_SPEED, PARAM_SPAWN_RATE, PARAM_TIME_SCALE, PARAM_CYCLE})
    {

        this->randomMode = random;
    }
};

class RainbowAnimation : public StripAnimation
{
public:
    void update();

    RainbowAnimation(StripState *state, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_RAINBOW, {PARAM_SCROLL_SPEED, PARAM_TIME_SCALE, PARAM_RAINBOW_REPEAT, PARAM_RAINBOW_OFFSET, PARAM_BRIGHTNESS})
    {
    }
};

class RandomAnimation : public StripAnimation
{
public:
    void update();
    RandomAnimation(StripState *state, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_RANDOM, {PARAM_SCROLL_SPEED, PARAM_TIME_SCALE, PARAM_RANDOM_OFF, PARAM_BRIGHTNESS})
    {
    }
};

class DoubleRainbowAnimation : public StripAnimation
{

public:
    void update();
    DoubleRainbowAnimation(StripState *state, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_DOUBLE_RAINBOW, {PARAM_SCROLL_SPEED, PARAM_RAINBOW_REPEAT, PARAM_RAINBOW_OFFSET, PARAM_BRIGHTNESS})
    {
    }
};

class SliderAnimation : public StripAnimation
{

public:
    void update();
    SliderAnimation(StripState *state, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_SLIDER, {PARAM_SLIDER_POSITION, PARAM_SLIDER_WIDTH, PARAM_SLIDER_REPEAT, PARAM_BRIGHTNESS, PARAM_HUE})
    {
    }
};

class FallingBricksAnimation : public StripAnimation
{
    float brickPos = -1;
    int stackHeight = 0;

public:
    void update();
    FallingBricksAnimation(StripState *state, int startLED, int endLED)
        : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_FALLING_BRICKS,
                          {PARAM_PARTICLE_WIDTH, PARAM_VELOCITY, PARAM_HUE,
                           PARAM_HUE_END, PARAM_HUE_VARIANCE, PARAM_BRIGHTNESS,
                           PARAM_TIME_SCALE, PARAM_DIRECTION})
    {
    }
};

class NebulaAnimation : public StripAnimation
{
    float noiseOffset = 0;

public:
    void update();
    NebulaAnimation(StripState *state, int startLED, int endLED) : StripAnimation(state, startLED, endLED, ANIMATION_TYPE_NEBULA, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_NOISE_SCALE, PARAM_NOISE_SPEED, PARAM_TIME_SCALE})
    {
    }
};