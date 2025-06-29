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
void ParticleAnimation::fadeParticleTail(float position, float width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{
    int start = floor(position - direction * (width - 1));
    int end = ceil(position);

    for (int i = start; direction > 0 ? i <= end : i >= end; i += direction)
    {
        if (i < 0 || i >= numLEDs())
            continue;

        float dist = (position - i * direction); // distance from head
        float t = dist / width;

        if (t < 0.0 || t > 1.0)
            continue;

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

        float adjustedBrightness = brightness * fade;

        float hue = interpolate(hueStart, hueEnd, t);
        colorFromHSV(animationColor, hue / 360.0, 1.0, adjustedBrightness / 255.0);
        setPixel(i, animationColor);
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
            particle->velocity += particle->acceleration;
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

            int width = particle->width;

            if (width < 1)
            {
                width = 1;
            }
            if (width > particle->width)
            {
                width = particle->width;
            }

            if (particle->velocity < -particle->maxSpeed)
            {
                particle->velocity = -particle->maxSpeed;
            }
            if (particle->velocity > particle->maxSpeed)
            {
                particle->velocity = particle->maxSpeed;
            }
            if (particle->randomDrift > 0)
            {
                if (random(0, 100) < particle->randomDrift)
                {
                    particle->acceleration = -particle->acceleration;
                }
            }

            if (particle->position < -width)
            {
                if (cycle)
                    particle->position = numLEDs() + width;
                else
                    particle->active = false;
            }
            else if (particle->position >= numLEDs() + width)
            {
                if (cycle)
                    particle->position = -width;
                else
                    particle->active = false;
            }
            else
            {

                int direction = particle->velocity > 0 ? 1 : -1;

                fadeParticleTail(particle->position, width, particle->hueStart, particle->hueEnd, particle->brightness, particle->fade, direction);
            }
        }
    }
}
void ParticleAnimation::spawnParticle()
{
    int width = getInt(PARAM_PARTICLE_WIDTH);
    float velocity = getFloat(PARAM_VELOCITY);
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int life = getInt(PARAM_PARTICLE_LIFE);
    if (random(0, 100) < 50)
    {

        spawnParticle(numLEDs(), -velocity, hueStart, hueEnd, brightness, width, life);
        return;
    }
    else
        spawnParticle(0, velocity, hueStart, hueEnd, brightness, width, life);
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
    float repeat = getFloat(PARAM_RAINBOW_REPEAT);
    float offset = getInt(PARAM_RAINBOW_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed;

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

    if (timeScale != 0)
    {
        int ranVal = random(0, 100);
        if (ranVal < spawnRate)
        {

            spawnParticle();
        }
    }

    if (randomMode)
    {
        updateRandomParticles();
    }
    else
    {
        updateParticles();
    }
}

void RandomAnimation::update()

{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    int randomOff = getInt(PARAM_RANDOM_OFF);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed;

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
    int position = numLEDs() / 2;
    int width = getInt(PARAM_SLIDER_WIDTH);
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
    float repeat = getFloat(PARAM_RAINBOW_REPEAT);
    float offset = getInt(PARAM_RAINBOW_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed;

    for (int i = 0; i < numLEDs(); i++)
    {
        int val = i + scrollPos;
        float hue = fmod(offset + (float(val) / float(numLEDs())) * repeat * 360.0, 360.0);
        colorFromHSV(animationColor, hue / 360.0, 1, brightness / 255.0);
        setPixel(i, animationColor);
    }
}