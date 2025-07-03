// animations.cpp
#include "animations.h"
#include "shared.h"
#include "stripState.h"

led animationColor;
int animCount = 0;

float clamp(float value, float min, float max)
{
    return std::max(min, std::min(max, value));
}
void StripAnimation::setPixel(int index, led color)
{

    stripState->setPixel(index + startLED, color);
}

void ParticleAnimation::updateRandomParticles()
{

    float startHue = getFloat(PARAM_HUE);
    float endHue = getFloat(PARAM_HUE_END);
    float velocity = getFloat(PARAM_VELOCITY);

    if (random(0, 100) > 90)
    {
        float vel = random(10, 50) / 10.0;
        if (abs(vel) < 0.5)
        {
            vel = random(1, 2);
        }
        // random(0, 360), random(0, 255)
        int shue = startHue + random(-60, 60);
        int ehue = startHue + random(-60, 60);
        int size = random(2, 10);
        spawnParticle(0, vel, shue, ehue, 200, size, 60);
    }
    updateParticles();
}
void ParticleAnimation::fadeParticleTail(float position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{

    // Serial.printf("Fading particle tail at position %g width %d hueStart %d hueEnd %d brightness %d fadeSpeed %f direction %d\n", position, width, hueStart, hueEnd, brightness, fadeSpeed, direction);
    for (int i = 0; i < width; i++)
    {
        int fadePos = position - i * direction;
        // Serial.printf("fade %d\n", fadePos);
        if (fadePos < 0 || fadePos > numLEDs())
        {

            continue;
        }

        float t = (float)i / (float)width;

        if (t < 0.0f || t > 1.0f)
        {
            Serial.printf("Invalid fade time %f for particle tail %g %g\n", t, i, width);
            continue;
        }

        float fade;
        if (t < 0.2f)
        {
            // Head: fade in quickly
            fade = t / 0.2f;
        }
        else
        {
            // Tail: fade out smoothly
            fade = 1.0f - ((t - 0.2f) / 0.8f);
            fade = pow(fade, fadeSpeed); // apply fade curve
        }
        // Serial.printf("Fade position %d t %f fade %f\n", fadePos, t, fade);
        float adjustedBrightness = brightness * fade;

        // Add randomness in the tail
        if (t >= 0.2f)
        {
            adjustedBrightness *= 0.9f + 0.2f * random(0, 1000) / 1000.0f; // ±10%
        }

        float hue = interpolate(hueStart, hueEnd, t);
        // Serial.printf("Fading particle tail at index %d with hue %f brightness %f\n", fadePos, hue, adjustedBrightness);
        colorFromHSV(animationColor, hue / 360.0, 1.0, adjustedBrightness / 255.0);
        setPixel(fadePos, animationColor);
    }
}
void ParticleAnimation::updateParticles()
{

    float timeScale = getFloat(PARAM_TIME_SCALE);
    bool cycle = getBool(PARAM_CYCLE);

    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int fade = getInt(PARAM_PARTICLE_FADE);
    int width = getInt(PARAM_PARTICLE_WIDTH);
    int life = getInt(PARAM_PARTICLE_LIFE);
    int randomDrift = getInt(PARAM_RANDOM_DRIFT);
    float acceleration = getFloat(PARAM_ACCELERATION);
    float maxSpeed = getFloat(PARAM_MAX_SPEED);

    if (animCount++ > 1000)
    {
        if (isVerbose())
            Serial.printf("Particle animation time scale: %f cycle: %d hueStart: %d hueEnd: %d brightness: %d fade: %d width: %d life: %d randomDrift: %d acceleration: %f maxSpeed: %f\n", timeScale, cycle, hueStart, hueEnd, brightness, fade, width, life, randomDrift, acceleration, maxSpeed);
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

            particle->position += particle->velocity * timeScale / 10.0;
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
            //     if (random(0, 100) < particle->randomDrift)
            //     {
            //         particle->acceleration = -particle->acceleration;
            //     }
            // }

            if (particle->position < -width)
            {
                if (cycle)
                    particle->position = numLEDs() + width;
                else
                    particle->active = false;
            }
            else if (particle->position > numLEDs() + width)
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
    int width = getInt(PARAM_PARTICLE_WIDTH);
    float timeScale = getFloat(PARAM_TIME_SCALE);
    float velocity = getFloat(PARAM_VELOCITY);
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int life = getInt(PARAM_PARTICLE_LIFE);

    if (random(0, 100) < 50)
    {
        int pos = numLEDs() + width;
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
    float repeat = getFloat(PARAM_RAINBOW_REPEAT);
    float offset = getInt(PARAM_RAINBOW_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed * timescale / 10.0;

    for (int i = 0; i < numLEDs(); i++)
    {
        int val = i + scrollPos;
        float hue = fmod(offset + (float(val) / float(numLEDs())) * repeat * 360.0, 360.0);
        colorFromHSV(animationColor, hue / 360.0, 1, brightness / 255.0);
        setPixel(i, animationColor);
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
        int ranVal = random(0, 1000);
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
        Serial.printf("Random animation scroll speed: %f random off: %d brightness: %d\n", scrollSpeed, randomOff, brightness);
        animCount = 0;
    }
    for (int i = 0; i < numLEDs(); i++)
    {
        int val = i + scrollPos;
        if (random(0, 100) > randomOff)
        {
            colorFromHSV(animationColor, float(val) / float(numLEDs()), 1, brightness / 255.0);
            setPixel(i, animationColor);
        }
    }
}

void SliderAnimation::update()
{
    //  slider animation is just a gradient that positioned in the middle of the strip with a width and hue and repeat factor
    int position = numLEDs() / 2 + getInt(PARAM_SLIDER_POSITION);
    float width = getFloat(PARAM_SLIDER_WIDTH) * numLEDs();
    float repeat = getFloat(PARAM_SLIDER_REPEAT);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int hue = getInt(PARAM_HUE);

    for (int i = 0; i < numLEDs(); i++)
    {
        int val = i - position;
        if (val < 0)
            val = -val;

        if (val > width / 2)
        {
            setPixel(i, {0, 0, 0}); // turn off pixel
        }
        else
        {
            float t = float(val) / float(width / 2);
            float hueValue = fmod(hue + t * repeat * 360.0, 360.0);
            colorFromHSV(animationColor, hueValue / 360.0, 1, brightness / 255.0);
            setPixel(i, animationColor);
        }
    }
}

void DoubleRainbowAnimation::update()
{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    float timescale = getFloat(PARAM_TIME_SCALE);
    float repeat = getFloat(PARAM_RAINBOW_REPEAT);
    float offset = getInt(PARAM_RAINBOW_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed * timescale;

    for (int i = 0; i < numLEDs(); i++)
    {
        int val = i + scrollPos;
        float hue = fmod(offset + (float(val) / float(numLEDs())) * repeat * 360.0, 360.0);
        colorFromHSV(animationColor, hue / 360.0, 1, brightness / 255.0);
        setPixel(i, animationColor);
    }
}