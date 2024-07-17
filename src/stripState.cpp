

#include "FastLED.h"
#include "stripState.h"

int val = 0;
int minr = 0;
int maxr = 0;

led color;

StripState::StripState(ParameterManager *parameterManager, CRGB row[], LED_STATE state, int numLEDS, int LED_PIN, int STRIP_INDEX, bool invert) : leds(row), ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX), invertLEDs(invert)

{
    this->parameterManager = parameterManager;

    leds = row;

    ledState = state;
}

void StripState::toggleMode()
{
    ledState = (LED_STATE)(((int)ledState + 1) % LED_STATE_COUNT);
    Serial.println("LED State: ");
    Serial.println(getLedStateName(ledState));
    this->clearPixels();
}

void StripState::updateRandomParticles()
{

    float startHue = parameterManager->getValue(PARAM_HUE);
    float endHue = parameterManager->getValue(PARAM_HUE_END);
    float velocity = parameterManager->getValue(PARAM_VELOCITY);

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

void StripState::fadeParticleTail(int position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{
    for (int j = 0; j < width; j++)
    {
        int index = position - j * direction;
        if (index >= 0 && index < numLEDS)
        {
            const int hue = interpolate(hueStart, hueEnd, (float)j / width);

            // Logarithmic decay for brightness
            float fadeFactor = log(1 + fadeSpeed * j) / log(1 + fadeSpeed * width);
            int adjustedBrightness = brightness * (1.0 - fadeFactor);

            colorFromHSV(color, hue / 360.0, 1.0, adjustedBrightness / 255.0);
            setPixel(index, color);
        }
    }
}
void StripState::updateParticles()
{

    float timeScale = parameterManager->getFloat(PARAM_TIME_SCALE);
    bool cycle = parameterManager->getBool(PARAM_CYCLE);

    if (parameterManager->getBool(PARAM_PARTICLE_UPDATE_ALL) && parameterManager->parameterChanged())
    {

        int hueStart = parameterManager->getValue(PARAM_HUE);
        int hueEnd = parameterManager->getValue(PARAM_HUE_END);
        int brightness = parameterManager->getValue(PARAM_BRIGHTNESS);
        int fade = parameterManager->getValue(PARAM_PARTICLE_FADE);
        int width = parameterManager->getValue(PARAM_PARTICLE_WIDTH);
        int life = parameterManager->getValue(PARAM_PARTICLE_LIFE);
        int randomDrift = parameterManager->getValue(PARAM_RANDOM_DRIFT);
        int acceleration = parameterManager->getValue(PARAM_ACCELERATION);
        int maxSpeed = parameterManager->getValue(PARAM_MAX_SPEED);
        int velocity = parameterManager->getValue(PARAM_VELOCITY);

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
                particle->acceleration = acceleration;
                particle->maxSpeed = maxSpeed;
                particle->velocity = velocity;
            }
        }
        // update all particles
    }

    for (int i = 0; i < 10; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {

            if (particle->randomDrift != 0)
            {
                // roll the dice to see if it should switch direction of acceleration
                if (random(0, 100) > particle->randomDrift)
                {
                    particle->acceleration = -particle->acceleration;
                }
            }

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
            if (particle->maxSpeed != 0)
            {
                width = (int)(particle->width * abs(particle->velocity) / particle->maxSpeed);
            }
            if (width < 1)
            {
                width = 1;
            }
            if (width > particle->width)
            {
                width = particle->width;
            }

            if (particle->velocity < -10)
            {
                particle->velocity = -10;
            }
            if (particle->velocity > 10)
            {
                particle->velocity = 10;
            }
            if (particle->randomDrift > 0)
            {
                if (random(0, 100) < particle->randomDrift / 10.0)
                {
                    particle->acceleration = -particle->acceleration;
                }
            }

            if (particle->position < -width)
            {
                if (cycle)
                    particle->position = numLEDS + width;
                else
                    particle->active = false;
            }
            else if (particle->position >= numLEDS + width)
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
String StripState::getStripState()
{
    return getLedStateName(ledState);
}
void StripState::update()
{

    // Depending on what ledState is, you'll update the LEDs accordingly
    if (isVerbose())
    {
        // Serial.println("Updating LEDs" + String(getLedStateName(ledState)));
    }

    switch (ledState)
    {
    case LED_STATE_IDLE:
    {
        clearPixels();
    }

    break;
    case LED_STATE_PARTICLES:
    {

        clearPixels();
        int spawnRate = parameterManager->getValue(PARAM_SPAWN_RATE);
        float timeScale = parameterManager->getFloat(PARAM_TIME_SCALE);
        if (spawnRate > 0 && timeScale != 0)
        {
            int ranVal = random(0, 100);

            if (ranVal < spawnRate)
            {

                spawnParticle();
            }
        }
        updateParticles();
    }
    break;
    case LED_STATE_RANDOM_PARTICLES:
    {
        clearPixels();
        updateRandomParticles();
    }
    break;
    case LED_STATE_SLIDER:
    {

        int centerPos = 0;
        bool useGravity = parameterManager->getBool(PARAM_SLIDER_GRAVITY);
        bool centered = parameterManager->getBool(PARAM_CENTERED);
        int position = parameterManager->getValue(PARAM_SLIDER_POSITION);

        float multiplier = parameterManager->getFloat(PARAM_MULTIPLIER);
        int width = parameterManager->getValue(PARAM_SLIDER_WIDTH);
        int hueshift = parameterManager->getValue(PARAM_SLIDER_HUE);
        float repeat = parameterManager->getValue(PARAM_SLIDER_REPEAT);

        if (useGravity)
            centerPos = (position + gravityPosition) % numLEDS;
        else
            centerPos = position % numLEDS;

        for (int i = 0; i < numLEDS; i++)
        {
            int distanceFromCenter = abs(i - centerPos);
            // check for overflow distance since it is a loop
            if (distanceFromCenter > numLEDS / 2)
            {
                distanceFromCenter = numLEDS - distanceFromCenter;
            }
            if (centered)
            {
                distanceFromCenter = abs(i - numLEDS / 2 - centerPos);
            }
            if (invertLEDs)
            {
                distanceFromCenter = width - distanceFromCenter;
            }

            // If pixel is outside the width, clear it
            if (distanceFromCenter > width * multiplier)
            {
                clearPixel(i);
            }
            else
            {

                // Calculate the hue for each pixel, based on its position relative to the center
                // The hue cycles 'repeat' times over the width of the strip

                float hue = fmod(hueshift + distanceFromCenter * repeat * (360.0 / width), 360.0);
                float fade = 1 - (distanceFromCenter / (width * multiplier));
                colorFromHSV(color, hue / 360.0, 1, fade);
                setPixel(i, color);
            }
        }
    }
    break;

    case LED_STATE_RAINBOW:
    {
        float scrollSpeed = parameterManager->getFloat(PARAM_SCROLL_SPEED);
        scrollPos += scrollSpeed;

        for (int i = 0; i < numLEDS; i++)
        {

            int val = i + scrollPos;
            colorFromHSV(color, float(val) / float(numLEDS), 1, 1);
            setPixel(i, color);
        }
    }
    break;

    case LED_STATE_DOUBLE_RAINBOW:
    {
        float scrollSpeed = parameterManager->getFloat(PARAM_SCROLL_SPEED);
        scrollPos += scrollSpeed;

        for (int i = 0; i < numLEDS; i += 2)
        {
            int val = i + scrollPos;
            colorFromHSV(color, float(val) / float(numLEDS), 1, 1);
            setPixel(i, color);
        }

        // TODO: use a second slider to drive a second scrollSpeed and use that here
        for (int i = 1; i < numLEDS; i += 2)
        {
            int val = i + scrollPos;
            colorFromHSV(color, 1 - (float(val) / float(numLEDS)), 1, 1);
            setPixel(i, color);
        }
    }

    case LED_STATE_RANDOM:
    {
        int randomMin = parameterManager->getValue(PARAM_RANDOM_MIN);
        int randomMax = parameterManager->getValue(PARAM_RANDOM_MAX);
        int randomOn = parameterManager->getValue(PARAM_RANDOM_ON);
        int randomOff = parameterManager->getValue(PARAM_RANDOM_OFF);

        val = 0;
        minr = min(randomMin, randomMax);
        maxr = max(randomMin, randomMax);
        for (int i = 0; i < numLEDS; i++)
        {
            if (random(0, 100) > randomOn)
            {
                if (random(0, 100) > randomOff)
                {
                    val = random(minr, maxr);
                    colorFromHSV(color, float(val) / float(255), 1, 1);
                    setPixel(i, color);
                }
                else
                {
                    clearPixel(i);
                }
            }
        }
    }
    break;

    default:
        break;
    }
}

bool StripState::respondToText(String command)
{

    bool verbose = isVerbose();

    if (command.startsWith("menu:"))
    {
        String menuName = command.substring(5);

        if (menuName == "Particles")
        {

            ledState = LED_STATE_PARTICLES;
        }
        else if (menuName == "Random")
        {

            ledState = LED_STATE_RANDOM;
        }
        else if (menuName == "Slider")
        {

            ledState = LED_STATE_SLIDER;
        }
        else if (menuName == "Rainbow")
        {
            ledState = LED_STATE_RAINBOW;
        }
        else if (menuName == "Double Rainbow")
        {
            ledState = LED_STATE_DOUBLE_RAINBOW;
        }
        else if (menuName == "Idle")
        {

            ledState = LED_STATE_IDLE;
        }
        else if (menuName == "RndParticles")
        {
            ledState = LED_STATE_RANDOM_PARTICLES;
        }

        else
        {
            return false;
        }
        Serial.printf("Set LED State:%d  %s", ledState, getLedStateName(ledState));
        return true;
    }
    return false;
}

void StripState::clearPixels()
{

    for (int i = 0; i < numLEDS; i++)
    {
        clearPixel(i);
    }
}

void StripState::setPixel(int index, led color)
{

    if (invertLEDs)

        leds[numLEDS - index - 1] = CRGB(color.r, color.g, color.b);

    else
        leds[index] = CRGB(color.r, color.g, color.b);
}
void StripState::setPixel(int index, int r, int g, int b)
{

    if (invertLEDs)

        leds[numLEDS - index - 1] = CRGB(r, g, b);

    else
        leds[index] = CRGB(r, g, b);
}
void StripState::clearPixel(int index)
{
    bool invert = parameterManager->getBool(PARAM_INVERT);
    if (invert)
        leds[numLEDS - index - 1] = CRGB(0, 0, 0);
    else
        leds[index] = CRGB(0, 0, 0);
}
