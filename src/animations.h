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
    int start = -1; // first led in the parent strip -1 means beginning of the strip
    int end = -1;   // last led in the parent strip -1 means end of the strip

public:
    virtual void update() = 0;
    StripAnimation(StripState *state, int start, int end, ANIMATION_TYPE type, std::vector<ParameterID> params, std::map<ParameterID, float> paramOverrides = {})
        : ParameterManager(getAnimationName(type), params, paramOverrides), stripState(state), start(start), end(end)
    {
    }
    ANIMATION_TYPE getAnimationType()
    {
        return animationType;
    }
    int numLEDs()
    {
        return this->end - this->start + 1;
    }
    int getstart()
    {
        return start;
    }
    int getend()
    {
        return end;
    }
    Node3D getLEDPosition(int ledIndex);
    void setPixel(int index, led color);
    void setPixelHSV(int index, float hue, float saturation, float value);
    String describe();
};

#define NUM_PARTICLES 20
class ParticleAnimation : public StripAnimation
{
    Particle particles[NUM_PARTICLES];
    bool randomMode = false;

    /**
     * Draw a fading tail behind a particle.
     *
     * Uses fractional pixel blending so tails appear smooth even on
     * low-density LED strips.
     *
     * @param position   Current particle position in LED coordinates.
     * @param width      Length of the tail in LEDs.
     * @param hueStart   Hue at the head of the particle (degrees).
     * @param hueEnd     Hue at the end of the tail (degrees).
     * @param brightness Base brightness for the head (0-255).
     * @param fadeSpeed  Exponent controlling how quickly the tail fades.
     * @param direction  Direction of motion: 1 forward, -1 backward.
     */
    void fadeParticleTail(float position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction);

    void spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life);
    void spawnParticle();
    void updateRandomParticles();
    void updateParticles();

public:
    void update();

    ParticleAnimation(StripState *state, bool random, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_PARTICLES, {PARAM_HUE, PARAM_HUE_END, PARAM_VELOCITY, PARAM_BRIGHTNESS, PARAM_WIDTH, PARAM_LIFE, PARAM_FADE, PARAM_RANDOM_DRIFT, PARAM_ACCELERATION, PARAM_MAX_SPEED, PARAM_SPAWN_RATE, PARAM_TIME_SCALE, PARAM_CYCLE, PARAM_REVERSE}, paramOverrides),
                                                                                                                              randomMode(random)
    {

        this->randomMode = random;
    }
};

class RainbowAnimation : public StripAnimation
{
public:
    void update();

    RainbowAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_RAINBOW, {PARAM_SCROLL_SPEED, PARAM_TIME_SCALE, PARAM_REPEAT, PARAM_OFFSET, PARAM_BRIGHTNESS}, paramOverrides)

    {
    }
};

class SingleColorAnimation : public StripAnimation
{
public:
    void update();

    SingleColorAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_SINGLE_COLOR, {PARAM_HUE, PARAM_BRIGHTNESS}, paramOverrides)
    {
    }
};

class RandomAnimation : public StripAnimation
{
public:
    void update();
    RandomAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_RANDOM, {PARAM_SCROLL_SPEED, PARAM_TIME_SCALE, PARAM_RANDOM_OFF, PARAM_BRIGHTNESS}, paramOverrides)
    {
    }
};

class DoubleRainbowAnimation : public StripAnimation
{

public:
    void update();
    DoubleRainbowAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_DOUBLE_RAINBOW, {PARAM_SCROLL_SPEED, PARAM_REPEAT, PARAM_OFFSET, PARAM_BRIGHTNESS}, paramOverrides)

    {
    }
};

class SliderAnimation : public StripAnimation
{

public:
    void update();
    SliderAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_SLIDER, {PARAM_POSITION, PARAM_WIDTH, PARAM_REPEAT, PARAM_BRIGHTNESS, PARAM_HUE}, paramOverrides) {}
};

class FallingBricksAnimation : public StripAnimation
{
    struct Brick
    {
        float pos{-1000.f};
        int width{1};
        float hue{0.f};
    } brick;
    int stackHeight = 0;

public:
    void update();
    FallingBricksAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {})
        : StripAnimation(state, start, end, ANIMATION_TYPE_BRICKS,
                         {PARAM_WIDTH, PARAM_VELOCITY, PARAM_HUE,
                          PARAM_HUE_END, PARAM_HUE_VARIANCE, PARAM_BRIGHTNESS,
                          PARAM_TIME_SCALE, PARAM_REVERSE},
                         paramOverrides)
    {
    }
};

class NebulaAnimation : public StripAnimation
{
    float noiseOffset = 0;

public:
    void update();
    NebulaAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {}) : StripAnimation(state, start, end, ANIMATION_TYPE_NEBULA, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_NOISE_SCALE, PARAM_NOISE_SPEED, PARAM_TIME_SCALE}, paramOverrides)
    {
    }
};

class SphereAnimation : public StripAnimation
{
public:
    void update();
    SphereAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {})
        : StripAnimation(state, start, end, ANIMATION_TYPE_SPHERE,
                         {PARAM_HUE, PARAM_BRIGHTNESS, PARAM_POS_X, PARAM_POS_Y, PARAM_POS_Z, PARAM_RADIUS, PARAM_THICKNESS},
                         paramOverrides)
    {
    }
};

class PlaneAnimation : public StripAnimation
{
public:
    void update();
    PlaneAnimation(StripState *state, int start, int end, std::map<ParameterID, float> paramOverrides = {})
        : StripAnimation(state, start, end, ANIMATION_TYPE_PLANE,
                         {PARAM_HUE, PARAM_BRIGHTNESS, PARAM_POS_Z, PARAM_THICKNESS},
                         paramOverrides)
    {
    }
};