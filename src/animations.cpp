// animations.cpp
#include "animations.h"
#include "shared.h"

led tempColor;

void ParticleAnimation::updateRandomParticles()
{

    float startHue = getParam(PARAM_HUE);
    float endHue = getParam(PARAM_HUE_END);
    float velocity = getParam(PARAM_VELOCITY);

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

void ParticleAnimation::fadeParticleTail(int position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{
    for (int j = 0; j < width; j++)
    {
        int index = position - j * direction;
        if (index >= 0 && index < stripState->getNumLEDS())
        {
            const int hue = interpolate(hueStart, hueEnd, (float)j / width);

            // Logarithmic decay for brightness
            float fadeFactor = log(1 + fadeSpeed * j) / log(1 + fadeSpeed * width);
            int adjustedBrightness = brightness * (1.0 - fadeFactor);

            colorFromHSV(tempColor, hue / 360.0, 1.0, adjustedBrightness / 255.0);
            stripState->setPixel(index, tempColor);
        }
    }
}
void ParticleAnimation::updateParticles()
{

    float timeScale = getParam(PARAM_TIME_SCALE);
    bool cycle = getParam(PARAM_CYCLE);

    int hueStart = getParam(PARAM_HUE);
    int hueEnd = getParam(PARAM_HUE_END);
    int brightness = getParam(PARAM_BRIGHTNESS);
    int fade = getParam(PARAM_PARTICLE_FADE);
    int width = getParam(PARAM_PARTICLE_WIDTH);
    int life = getParam(PARAM_PARTICLE_LIFE);
    int randomDrift = getParam(PARAM_RANDOM_DRIFT);
    int acceleration = getParam(PARAM_ACCELERATION);
    int maxSpeed = getParam(PARAM_MAX_SPEED);
    // int velocity = getValue(PARAM_VELOCITY);
    // %d %d %d %d %d %d %d %d %d\n", hueStart, hueEnd, brightness, fade, width, life, randomDrift, acceleration, maxSpeed, velocity);
    // Serial.printf("Update all particles hue start:%d \n  hue end:%d \n brightness:%d \n fade:%d \n width:%d \n life:%d \n randomDrift:%d \n acceleration:%d \n maxSpeed:%d \n velocity:%d \n", hueStart, hueEnd, brightness, fade, width, life, randomDrift, acceleration, maxSpeed, velocity);
    for (int i = 0; i < 10; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {
            particle->hueStart = hueStart;
            particle->hueEnd = hueEnd;
            particle->brightness = brightness;
            particle->fade = fade;
            particle->width = width;
            particle->life = life;
            particle->randomDrift = randomDrift;
            // particle->acceleration = acceleration;
            // particle->maxSpeed = maxSpeed;
            // particle->velocity = velocity;
        }
    }
    // update all particles

    for (int i = 0; i < 10; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {

            particle->position += particle->velocity * timeScale;
            particle->velocity += particle->acceleration * timeScale;
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
                    particle->position = stripState->getNumLEDS() + width;
                else
                    particle->active = false;
            }
            else if (particle->position >= stripState->getNumLEDS() + width)
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
    int width = getParam(PARAM_PARTICLE_WIDTH);
    int velocity = getParam(PARAM_VELOCITY);
    int hueStart = getParam(PARAM_HUE);
    int hueEnd = getParam(PARAM_HUE_END);
    int brightness = getParam(PARAM_BRIGHTNESS);
    int life = getParam(PARAM_PARTICLE_LIFE);

    spawnParticle(0, velocity, hueStart, hueEnd, brightness, width, life);
};

void RainbowAnimation::update(StripState *strip)
{
    float scrollSpeed = getParam(PARAM_SCROLL_SPEED);
    float repeat = getParam(PARAM_RAINBOW_REPEAT);
    float offset = getParam(PARAM_RAINBOW_OFFSET);
    int brightness = getParam(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed;

    for (int i = 0; i < strip->getNumLEDS(); i++)
    {
        int val = i + scrollPos;
        float hue = fmod(offset + (float(val) / float(strip->getNumLEDS())) * repeat * 360.0, 360.0);
        colorFromHSV(tempColor, hue / 360.0, 1, brightness / 255.0);
        strip->setPixel(i, tempColor);
    }
}
void ParticleAnimation::update(StripState *strip)
{
    int spawnRate = getParam(PARAM_SPAWN_RATE);
    float timeScale = getParam(PARAM_TIME_SCALE);

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
