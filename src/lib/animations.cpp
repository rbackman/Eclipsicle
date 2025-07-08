// animations.cpp
#include "animations.h"
#include "shared.h"
#include "stripState.h"
#include <FastLED.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include "log.h"

led animationColor;
int animCount = 0;

static inline int randomInt(int minVal, int maxVal)
{
    return minVal + std::rand() % (maxVal - minVal);
}

float clamp(float value, float min, float max)
{
    return std::max(min, std::min(max, value));
}
void StripAnimation::setPixel(int index, led color)
{

    if (stripState->getAnimationCount() > 1)
    {
        stripState->blendPixel(index + start, color);
    }
    else
    {
        stripState->setPixel(index + start, color);
    }
}
void StripAnimation::blendPixel(int index, led color)
{

    stripState->blendPixel(index + start, color);
}
Vec3D StripAnimation::getLEDPosition(int ledIndex)
{
    if (ledIndex < 0 || ledIndex >= numLEDs())
    {
        LOG_PRINTF("Invalid LED index %d for strip animation\n", ledIndex);
        return Vec3D{};
    }
    return stripState->getNode3D(ledIndex + start);
}
void StripAnimation::setPixelHSV(int index, float hue, float saturation, float value)
{

    colorFromHSV(animationColor, hue, saturation, value);
    stripState->blendPixel(index, animationColor);
}

std::string StripAnimation::describe()
{
    std::ostringstream desc;
    desc << getAnimationName(animationType) << " start:" << start << " end:" << end;

    for (const auto &p : getIntParameters())
    {
        desc << " " << getParameterName(p.id) << ":" << p.value;
    }
    for (const auto &p : getFloatParameters())
    {
        desc << " " << getParameterName(p.id) << ":" << p.value;
    }
    for (const auto &p : getBoolParameters())
    {
        desc << " " << getParameterName(p.id) << ":" << (p.value ? "1" : "0");
    }
    return desc.str();
}

std::string StripAnimation::describeCompact()
{
    std::ostringstream desc;
    desc << getAnimationName(animationType) << " start:" << start << " end:" << end;
    for (const auto &p : getIntParameters())
    {
        desc << " " << p.id << ":" << p.value;
    }
    for (const auto &p : getFloatParameters())
    {
        desc << " " << p.id << ":" << p.value;
    }
    for (const auto &p : getBoolParameters())
    {
        desc << " " << p.id << ":" << (p.value ? "1" : "0");
    }
    return desc.str();
}

void ParticleAnimation::updateRandomParticles()
{

    float startHue = getFloat(PARAM_HUE);
    float endHue = getFloat(PARAM_HUE_END);
    float velocity = getFloat(PARAM_VELOCITY);

    if (randomInt(0, 100) > 90)
    {
        float vel = randomInt(10, 50) / 100.0;
        if (abs(vel) < 0.5)
        {
            vel = randomInt(1, 2);
        }
        // randomInt(0, 360), randomInt(0, 255)
        int shue = startHue + randomInt(-60, 60);
        int ehue = startHue + randomInt(-60, 60);
        int size = randomInt(2, 10);
        spawnParticle(0, vel, shue, ehue, 200, size, 60);
    }
    updateParticles();
}
void ParticleAnimation::fadeParticleTail(float position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{
    // Smoothly draw a fading trail behind the particle.
    float invWidth = 1.0f / std::max(width - 1, 1);
    float brightScale = brightness / 255.0f;
    const int ledCount = numLEDs();
    for (int i = 0; i < width; i++)
    {
        float fadePos = position - i * direction;

        float t = static_cast<float>(i) * invWidth;
        float fade = 1.0f - t;        // start bright at the head
        fade *= fade;                 // quadratic falloff
        fade = powf(fade, fadeSpeed); // user controlled exponent

        float hue = interpolate(hueStart, hueEnd, t) / 360.0f;
        float value = brightScale * clamp(fade, 0.0f, 1.0f);

        // Fractional pixel rendering for smoother output.
        int lower = floor(fadePos);
        int upper = lower + 1;
        float frac = fadePos - lower;

        if (lower >= 0 && lower < ledCount)
        {
            led temp;
            colorFromHSV(temp, hue, 1.0f, value * (1.0f - frac));
            blendPixel(lower, temp);
        }
        if (upper >= 0 && upper < ledCount)
        {
            led temp;
            colorFromHSV(temp, hue, 1.0f, value * frac);
            blendPixel(upper, temp);
        }
    }
}
void ParticleAnimation::updateParticles()
{

    float timeScale = getFloat(PARAM_TIME_SCALE);
    bool cycle = getBool(PARAM_CYCLE);
    const int ledCount = numLEDs();

    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int fade = getInt(PARAM_FADE);
    int width = getInt(PARAM_WIDTH);
    int life = getInt(PARAM_LIFE);
    int randomDrift = getInt(PARAM_RANDOM_DRIFT);
    float acceleration = getFloat(PARAM_ACCELERATION);
    float maxSpeed = getFloat(PARAM_MAX_SPEED);

    if (animCount++ > 1000)
    {
        if (isVerbose())
            LOG_PRINTF("Particle animation time scale: %f cycle: %d hueStart: %d hueEnd: %d brightness: %d fade: %d width: %d life: %d randomDrift: %d acceleration: %f maxSpeed: %f\n", timeScale, cycle, hueStart, hueEnd, brightness, fade, width, life, randomDrift, acceleration, maxSpeed);
        animCount = 0;
    }
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {
            particle->hueStart = hueStart;
            particle->hueEnd = hueEnd;
            particle->brightness = brightness;
            particle->fade = fade / 100.0; // convert to 0-1 range
            particle->width = width;
            particle->life = life;
            particle->randomDrift = randomDrift;
            // particle->acceleration = acceleration;
            // particle->maxSpeed = maxSpeed;
            // particle->velocity = velocity;
        }
    }
    // update all particles

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {

            particle->position += particle->velocity * timeScale / 100.0;
            particle->velocity += particle->velocity * particle->acceleration;
            if (particle->velocity > particle->maxSpeed)
            {
                particle->velocity = particle->maxSpeed;
            }
            if (particle->velocity < -particle->maxSpeed)
            {
                particle->velocity = -particle->maxSpeed;
            }

            if (particle->life != -1 && timeScale != 0)
            {
                particle->life--;
                if (particle->life <= 0)
                {
                    particle->active = false;
                }
            }

            // if (particle->randomDrift > 0)
            // {
            //     if (randomInt(0, 100) < particle->randomDrift)
            //     {
            //         particle->acceleration = -particle->acceleration;
            //     }
            // }

            if (particle->position < -width)
            {
                if (cycle)
                    particle->position = ledCount + width;
                else
                    particle->active = false;
            }
            else if (particle->position > ledCount + width)
            {
                if (cycle)
                    particle->position = -width;
                else
                    particle->active = false;
            }

            // if (particle->velocity < 0)
            // {
            //     Serial.printf("Particle %d is moving backwards with position  %g\n", i, particle->position);
            // }
            int direction = particle->velocity > 0 ? 1 : -1;

            fadeParticleTail(particle->position, width, particle->hueStart, particle->hueEnd, particle->brightness, particle->fade, direction);
        }
    }
}
void ParticleAnimation::spawnParticle()
{
    int width = getInt(PARAM_WIDTH);
    float timeScale = getFloat(PARAM_TIME_SCALE);
    float velocity = getFloat(PARAM_VELOCITY);
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int life = getInt(PARAM_LIFE);

    const int ledCount = numLEDs();

    if (randomInt(0, 100) < 50)
    {
        int pos = ledCount + width;
        float vel = -velocity;
        // Serial.printf("Spawning particle at end with position %d velocity %f\n", pos, vel);
        spawnParticle(pos, vel, hueStart, hueEnd, brightness, width, life);
    }
    else
    {
        int pos = -width;
        float vel = velocity;
        spawnParticle(pos, vel, hueStart, hueEnd, brightness, width, life);
    }
};
void ParticleAnimation::spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life)
{

    bool used = false;
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        if (!particles[i].active)
        {
            used = true;
            particles[i].active = true;
            particles[i].position = position;
            particles[i].velocity = velocity;
            particles[i].hueStart = hueStart;
            particles[i].hueEnd = hueEnd;
            particles[i].brightness = brightness;
            particles[i].width = width;
            particles[i].life = life;

            // if (isVerbose())
            // {
            // Serial.printf("\nSpawned particle\n  position x: %d\n velocity: %f \nhueStart: %d \nhueEnd: %d \nbrightness: %d \nwidth: %d \nlife: %d", position, velocity, hueStart, hueEnd, brightness, width, life);
            // }

            return;
        }
    }
    if (!used)
    {
        // Serial.println("No free particles");
    }
}

void RainbowAnimation::update()
{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    float timescale = getFloat(PARAM_TIME_SCALE);
    float repeat = getFloat(PARAM_REPEAT);
    float offset = getInt(PARAM_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed * timescale / 100.0f;

    const int count = numLEDs();
    const float invCount = 1.0f / count;
    const float bright = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        float hue = fmodf(offset + (i + scrollPos) * invCount * repeat * 360.0f, 360.0f);
        setPixelHSV(i, hue / 360.0f, 1.0f, bright);
    }
}
void ParticleAnimation::update()
{
    int spawnRate = getInt(PARAM_SPAWN_RATE);
    float timeScale = getFloat(PARAM_TIME_SCALE);

    if (randomMode)
    {
        updateRandomParticles();
    }
    else
    {
        updateParticles();
    }
    if (timeScale != 0)
    {
        int ranVal = randomInt(0, 1000);
        if (ranVal < spawnRate)
        {

            spawnParticle();
        }
    }
}

void RandomAnimation::update()

{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    float timeScale = getFloat(PARAM_TIME_SCALE);
    int randomOff = getInt(PARAM_RANDOM_OFF);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed * timeScale;

    if (animCount++ > 100)
    {
        LOG_PRINTF("Random animation scroll speed: %f random off: %d brightness: %d\n", scrollSpeed, randomOff, brightness);
        animCount = 0;
    }

    const int count = numLEDs();
    const float invCount = 1.0f / count;
    const float bright = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        int val = i + scrollPos;
        if (randomInt(0, 100) > randomOff)
        {
            float hue = fmodf(float(val) * invCount * 360.0f, 360.0f);
            setPixelHSV(i, hue / 360.0f, 1.0f, bright);
        }
    }
}

void SliderAnimation::update()
{
    //  slider animation is just a gradient that positioned in the middle of the strip with a width and hue and repeat factor
    const int count = numLEDs();
    int position = count / 2 + getInt(PARAM_POSITION);
    int width = getInt(PARAM_WIDTH) * count;
    float repeat = getFloat(PARAM_REPEAT);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int hue = getInt(PARAM_HUE);

    const float bright = brightness / 255.0f;
    int half = width / 2;
    for (int i = 0; i < count; i++)
    {
        int val = i - position;
        if (val < 0)
            val = -val;

        if (val > half)
        {
            setPixel(i, {0, 0, 0}); // turn off pixel
        }
        else
        {
            float t = half > 0 ? float(val) / float(half) : 0.0f;
            float hueValue = fmodf(hue + t * repeat * 360.0f, 360.0f);

            setPixelHSV(i, hueValue / 360.0f, 1.0f, bright);
        }
    }
}

void DoubleRainbowAnimation::update()
{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    float timescale = getFloat(PARAM_TIME_SCALE);
    float repeat = getFloat(PARAM_REPEAT);
    float offset = getInt(PARAM_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed * timescale;

    const int count = numLEDs();
    const float invCount = 1.0f / count;
    const float bright = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        float hue = fmodf(offset + (i + scrollPos) * invCount * repeat * 360.0f, 360.0f);
        setPixelHSV(i, hue / 360.0f, 1.0f, bright);
    }
}
void FallingBricksAnimation::update()
{
    int width = getInt(PARAM_WIDTH);
    float speed = getFloat(PARAM_VELOCITY);
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int hueVar = getInt(PARAM_HUE_VARIANCE);
    int brightness = getInt(PARAM_BRIGHTNESS);
    float timeScale = getFloat(PARAM_TIME_SCALE);
    bool reverse = getBool(PARAM_REVERSE);

    const int count = numLEDs();
    const float bright = brightness / 255.0f;
    auto mapIdx = [&](int idx)
    { return reverse ? count - 1 - idx : idx; };

    int maxBricks = std::max(1, count / width);

    // Spawn a new brick when the current one is inactive
    if (brick.pos < -width && stackHeight < count)
    {
        brick.width = width;
        int brickIndex = stackHeight / width;
        float t = float(brickIndex) / float(maxBricks - 1);
        float baseHue = interpolate(hueStart, hueEnd, t);
        float n = ((inoise8(brickIndex * 50) / 255.0f) * 2.0f - 1.0f) * hueVar;
        brick.hue = fmod(baseHue + n + 360.0f, 360.0f);
        brick.pos = reverse ? -width : count - 1 + width;
    }

    // Move the falling brick
    if ((reverse && brick.pos < count) || (!reverse && brick.pos >= 0))
    {
        brick.pos += (reverse ? 1 : -1) * speed * timeScale / 100.0f;

        // Determine when it lands
        bool landed = reverse
                          ? brick.pos + (width - 1) >= count - 1 - stackHeight
                          : brick.pos - (width - 1) <= stackHeight;

        if (landed)
        {
            stackHeight += width;
            // Mark brick as inactive by positioning it below the spawn
            // threshold so a new one will spawn on the next frame.
            brick.pos = -width - 1;

            if (stackHeight >= count)
                stackHeight = 0;
        }
    }

    // Draw stacked bricks
    for (int i = 0; i < stackHeight && i < count; i++)
    {
        int brickIndex = i / width;
        float t = float(brickIndex) / float(maxBricks - 1);
        float baseHue = interpolate(hueStart, hueEnd, t);
        float n = ((inoise8(brickIndex * 50) / 255.0f) * 2.0f - 1.0f) * hueVar;
        float brickHue = fmod(baseHue + n + 360.0f, 360.0f);

        setPixelHSV(mapIdx(i), brickHue / 360.0f, 1.0f, bright);
    }

    // Draw falling brick
    if (brick.pos >= 0 && brick.pos < count)
    {
        for (int i = 0; i < width; i++)
        {
            int idx = reverse ? (int)brick.pos + i : (int)brick.pos - i;
            if (idx >= 0 && idx < count)
            {
                // colorFromHSV(animationColor, brick.hue / 360.0f, 1.0f,
                //              brightness / 255.0f);
                // setPixel(idx, animationColor);
                setPixelHSV(mapIdx(idx), brick.hue / 360.0f, 1.0f, bright);
            }
        }
    }
}

void NebulaAnimation::update()
{
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    float scale = getFloat(PARAM_NOISE_SCALE);
    float speed = getFloat(PARAM_NOISE_SPEED);
    float timeScale = getFloat(PARAM_TIME_SCALE);

    noiseOffset += speed * timeScale / 100.0f;

    const int count = numLEDs();
    const float invCount = 1.0f / count;
    const float brightScale = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        float t = float(i) * invCount;
        float baseHue = interpolate(hueStart, hueEnd, t);
        uint8_t noiseVal = inoise8(i * scale * 20, int(noiseOffset * 100));
        float hue = fmodf(baseHue + (noiseVal / 255.0f) * 60.0f, 360.0f);
        float bright = brightScale * powf(noiseVal / 255.0f, 3.0f);
        setPixelHSV(i, hue / 360.0f, 1.0f, bright);
    }
}

void SingleColorAnimation::update()
{
    int hue = getInt(PARAM_HUE);
    int brightness = getInt(PARAM_BRIGHTNESS);
    float hueValue = hue / 360.0f;               // convert to 0-1 range
    float brightnessValue = brightness / 255.0f; // convert to 0-1 range
    colorFromHSV(animationColor, hueValue, 1.0f, brightnessValue);

    const int count = numLEDs();
    for (int i = 0; i < count; i++)
    {
        setPixel(i, animationColor);
    }
}

void SphereAnimation::update()
{
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    float cx = getFloat(PARAM_POS_X);
    float cy = getFloat(PARAM_POS_Y);
    float cz = getFloat(PARAM_POS_Z);
    float radius = getFloat(PARAM_RADIUS);
    float thick = getFloat(PARAM_THICKNESS);

    int insphereCount = 0;
    const int count = numLEDs();
    const float bright = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        auto pos = getLEDPosition(i);
        float dx = pos.x - cx;
        float dy = pos.y - cy;
        float dz = pos.z - cz;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        float delta = fabs(dist - radius);
        float t = 1.0f - clamp(thick > 0 ? delta / thick : 1.0f, 0.0f, 1.0f);
        if (t <= 0.0f)
            continue;
        insphereCount++;
        float hue = interpolate(hueStart, hueEnd, t) / 360.0f;
        setPixelHSV(i, hue, 1.0f, bright);
    }
}

void PlaneAnimation::update()
{
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    float planeY = getFloat(PARAM_POS_Y);
    float thick = getFloat(PARAM_THICKNESS);

    const int count = numLEDs();
    const float bright = brightness / 255.0f;

    for (int i = 0; i < count; i++)
    {
        auto pos = getLEDPosition(i);
        float delta = fabs(pos.y - planeY);
        float t = 1.0f - clamp(thick > 0 ? delta / thick : 1.0f, 0.0f, 1.0f);
        if (t <= 0.0f)
            continue;
        float hue = interpolate(hueStart, hueEnd, t) / 360.0f;
        setPixelHSV(i, hue, 1.0f, bright);
    }
}
