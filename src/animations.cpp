// animations.cpp
#include "animations.h"
#include "shared.h"

led animationColor;
int animCount = 0;
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

            colorFromHSV(animationColor, hue / 360.0, 1.0, adjustedBrightness / 255.0);
            stripState->setPixel(index, animationColor);
        }
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

            particle->position += particle->velocity * timeScale / 100.0;
            particle->velocity += particle->acceleration * timeScale / 100.0;
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
    int width = getInt(PARAM_PARTICLE_WIDTH);
    float velocity = getFloat(PARAM_VELOCITY);
    int hueStart = getInt(PARAM_HUE);
    int hueEnd = getInt(PARAM_HUE_END);
    int brightness = getInt(PARAM_BRIGHTNESS);
    int life = getInt(PARAM_PARTICLE_LIFE);

    spawnParticle(0, velocity, hueStart, hueEnd, brightness, width, life);
};
void ParticleAnimation::spawnParticle(int position, float velocity, int hueStart, int hueEnd, int brightness, int width, int life)
{

    bool used = false;
    for (int i = 0; i < 10; i++)
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

void RainbowAnimation::update(StripState *strip)
{
    float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
    float repeat = getFloat(PARAM_RAINBOW_REPEAT);
    float offset = getInt(PARAM_RAINBOW_OFFSET);
    int brightness = getInt(PARAM_BRIGHTNESS);

    scrollPos += scrollSpeed;
    if (animCount++ > 100)
    {
        if (isVerbose())
        {
            Serial.printf("Rainbow animation scroll speed: %f repeat: %f offset: %f brightness: %d\n", scrollSpeed, repeat, offset, brightness);
        }
        animCount = 0;
    }
    for (int i = 0; i < strip->getNumLEDS(); i++)
    {
        int val = i + scrollPos;
        float hue = fmod(offset + (float(val) / float(strip->getNumLEDS())) * repeat * 360.0, 360.0);
        colorFromHSV(animationColor, hue / 360.0, 1, brightness / 255.0);
        strip->setPixel(i, animationColor);
    }
}
void ParticleAnimation::update(StripState *strip)
{
    int spawnRate = getInt(PARAM_SPAWN_RATE);
    float timeScale = getFloat(PARAM_TIME_SCALE);

    // if (animCount++ > 100)
    // {
    //     if (isVerbose())
    //     {
    //         Serial.printf("Spawn rate: %d timeScale: %f\n", spawnRate, timeScale);
    //     }
    //     animCount = 0;
    // }
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

void RandomAnimation::update(StripState *strip)
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
    for (int i = 0; i < strip->getNumLEDS(); i++)
    {
        int val = i + scrollPos;
        if (random(0, 100) > randomOff)
        {
            colorFromHSV(animationColor, float(val) / float(strip->getNumLEDS()), 1, brightness / 255.0);
            strip->setPixel(i, animationColor);
        }
    }
}