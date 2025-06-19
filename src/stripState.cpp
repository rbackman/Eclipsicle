
#ifdef USE_LEDS

#include "stripState.h"
#include "animations.h"
int val = 0;
int minr = 0;
int maxr = 0;
led tempColor;

StripState::StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX, bool invert) : ParameterManager(("Strip" + String(STRIP_INDEX + 1)).c_str(), {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE, PARAM_INVERT}), ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX), invertLEDs(invert)

{
    leds = new CRGB[numLEDS];
    if (state != LED_STATE_IDLE)
    {
        Serial.print("Starting strip ");
        Serial.print(stripIndex);
        Serial.print(" with state ");
        Serial.println(getLedStateName(state));
    }
}

void StripState::toggleMode()
{
    ledState = (LED_STATE)(((int)ledState + 1) % LED_STATE_COUNT);
    Serial.println("LED State: ");
    Serial.println(getLedStateName(ledState));
    this->clearPixels();
}

String StripState::getStripState()
{
    return getLedStateName(ledState);
}

void StripState::addAnimation(ANIMATION_TYPE anim)
{
    if (ledState == LED_STATE_SINGLE_ANIMATION)
    {
        animations.clear();
    }
    if (anim == ANIMATION_TYPE_PARTICLES)
    {
        ParticleAnimation particleAnimation(this, false);
        Serial.println("Adding particle animation");
        animations.emplace_back(std::make_unique<ParticleAnimation>(particleAnimation));
    }
    else if (anim == ANIMATION_TYPE_RAINBOW)
    {
        RainbowAnimation rainbowAnimation(this);
        Serial.println("Adding rainbow animation");
        animations.emplace_back(std::make_unique<RainbowAnimation>(rainbowAnimation));
    }
    else if (anim == ANIMATION_TYPE_DOUBLE_RAINBOW)
    {
    }
    else if (anim == ANIMATION_TYPE_SLIDER)
    {
    }
    else if (anim == ANIMATION_TYPE_RANDOM)
    {
        RandomAnimation randomAnimation(this);
        animations.emplace_back(std::make_unique<RandomAnimation>(randomAnimation));
    }
    else if (anim == ANIMATION_TYPE_IDLE)
    {
        // ParticleAnimation particleAnimation(this, false);
        // animations.emplace_back(std::make_unique<ParticleAnimation>(particleAnimation));
    }
    else if (anim == ANIMATION_TYPE_POINT_CONTROL)
    {
    }
    else if (anim == ANIMATION_TYPE_RANDOM_PARTICLES)
    {
        ParticleAnimation particleAnimation(this, true);
        animations.emplace_back(std::make_unique<ParticleAnimation>(particleAnimation));
    }
}
void StripState::update()
{
    clearPixels();
    if (beatSize > 0.1)
    {
        float beatFade = getFloat(PARAM_BEAT_FADE);
        int beatMaxSize = getInt(PARAM_BEAT_MAX_SIZE);

        beatSize = beatSize - beatFade;
        if (beatSize < 1)
        {
            beatSize = 0;
        }
        else if (beatSize > beatMaxSize)
        {
            beatSize = beatMaxSize;
        }
    }
    else
    {
        beatSize = 0;
    }
    switch (ledState)
    {
    case LED_STATE_IDLE:

        // clearPixels();

        break;

    case LED_STATE_SINGLE_ANIMATION:
    case LED_STATE_MULTI_ANIMATION:
    {
        if (animations.size() > 0)
        {
            if (ledState == LED_STATE_SINGLE_ANIMATION)
            {
                animations[currentAnimation].get()->update(this);
            }
            else
            {
                for (int i = 0; i < animations.size(); i++)
                {
                    animations[i].get()->update(this);
                }
            }
        }
    }

    break;

        break;
        // case LED_STATE_SLIDER:
        // {

        //     int centerPos = 0;
        //     bool useGravity = getBool(PARAM_SLIDER_GRAVITY);
        //     bool centered = getBool(PARAM_CENTERED);
        //     int position = getValue(PARAM_SLIDER_POSITION);

        //     float multiplier = 1; // getFloat(PARAM_SLIDER_MULTIPLIER);
        //     int width = getValue(PARAM_SLIDER_WIDTH) + beatSize;
        //     int hueshift = getValue(PARAM_SLIDER_HUE);
        //     float repeat = getFloat(PARAM_SLIDER_REPEAT);

        //     if (useGravity)
        //         centerPos = (position + gravityPosition) % numLEDS;
        //     else
        //         centerPos = position % numLEDS;

        //     clearPixels();

        //     int first = centerPos - width / 2;
        //     for (int i = first; i <= first + width; i++)
        //     {

        //         int distanceFromCenter = abs(i - centerPos);
        //         // check for overflow distance since it is a loop
        //         // if (distanceFromCenter > numLEDS / 2)
        //         // {
        //         //     distanceFromCenter = numLEDS - distanceFromCenter;
        //         // }
        //         if (centered)
        //         {
        //             distanceFromCenter = abs(i - numLEDS / 2 - centerPos);
        //         }
        //         if (invertLEDs)
        //         {
        //             distanceFromCenter = width - distanceFromCenter;
        //         }

        //         // // If pixel is outside the width, clear it
        //         // if (distanceFromCenter > width * multiplier)
        //         // {
        //         //     clearPixel(i);
        //         // }
        //         // else
        //         // {

        //         // Calculate the hue for each pixel, based on its position relative to the center
        //         // The hue cycles 'repeat' times over the width of the strip

        //         float hue = fmod(hueshift + distanceFromCenter * repeat * (360.0 / width), 360.0);
        //         float fade = 1 - (distanceFromCenter / (width * multiplier));
        //         colorFromHSV(tempColor, hue / 360.0, 1, fade);
        //         setPixel(i, tempColor);
        //         // }
        //     }
        // }
        // break;

        break;

        // case LED_STATE_DOUBLE_RAINBOW:
        // {
        //     float scrollSpeed = getFloat(PARAM_SCROLL_SPEED);
        //     scrollPos += scrollSpeed;

        //     for (int i = 0; i < numLEDS; i += 2)
        //     {
        //         int val = i + scrollPos;
        //         colorFromHSV(tempColor, float(val) / float(numLEDS), 1, 1);
        //         setPixel(i, tempColor);
        //     }

        //     // TODO: use a second slider to drive a second scrollSpeed and use that here
        //     for (int i = 1; i < numLEDS; i += 2)
        //     {
        //         int val = i + scrollPos;
        //         colorFromHSV(tempColor, 1 - (float(val) / float(numLEDS)), 1, 1);
        //         setPixel(i, tempColor);
        //     }
        // }
        // break;

        // case LED_STATE_RANDOM:
        // {
        //     int randomMin = getValue(PARAM_RANDOM_MIN);
        //     int randomMax = getValue(PARAM_RANDOM_MAX);
        //     int randomOn = getValue(PARAM_RANDOM_ON);
        //     int randomOff = getValue(PARAM_RANDOM_OFF);

        //     val = 0;
        //     minr = min(randomMin, randomMax);
        //     maxr = max(randomMin, randomMax);
        //     for (int i = 0; i < numLEDS; i++)
        //     {
        //         if (random(0, 100) > randomOn)
        //         {
        //             if (random(0, 100) > randomOff)
        //             {
        //                 val = random(minr, maxr);
        //                 colorFromHSV(tempColor, float(val) / float(255), 1, 1);
        //                 setPixel(i, tempColor);
        //             }
        //             else
        //             {
        //                 clearPixel(i);
        //             }
        //         }
        //     }
        // }
        // break;

    case LED_STATE_POINT_CONTROL:
    {

        int pointPosition = (int)getInt(PARAM_CURRENT_LED);

        int pointHue = getInt(PARAM_HUE);
        int pointBrightness = getInt(PARAM_BRIGHTNESS);
        clearPixels();

        if (isActive)
        {
            // Serial.printf("point control strip %s   %d \n", getName().c_str(), pointPosition);

            colorFromHSV(tempColor, float(pointHue) / float(255), 1, float(pointBrightness) / float(255));
            setPixel(pointPosition, tempColor);
        }
        else
        {

            clearPixels();
        }
    }
    break;
    default:
        break;
    }
}

void StripState::setAll(led tcol)
{
    for (int i = 0; i < numLEDS; i++)
    {
        setPixel(i, tcol);
    }
}
bool StripState::respondToText(String command)
{

    bool verbose = isVerbose();

    if (command.startsWith("menu:"))
    {
        String menuName = command.substring(5);
        menuName.trim();
        menuName.toLowerCase();
        ANIMATION_TYPE animType = ANIMATION_TYPE_NONE;
        if (menuName.startsWith("part"))
        {

            animType = ANIMATION_TYPE_PARTICLES;
        }
        else if (menuName.startsWith("rand") || menuName.startsWith("random"))
        {

            animType = ANIMATION_TYPE_RANDOM;
        }
        else if (menuName == "slider")
        {

            animType = ANIMATION_TYPE_SLIDER;
        }
        else if (menuName.startsWith("rain"))
        {

            animType = ANIMATION_TYPE_RAINBOW;
        }

        else if (menuName.startsWith("double"))
        {
            animType = ANIMATION_TYPE_DOUBLE_RAINBOW;
        }
        else if (menuName == "idle")
        {

            animType = ANIMATION_TYPE_IDLE;
        }
        else if (menuName == "rndparticles")
        {
            animType = ANIMATION_TYPE_RANDOM_PARTICLES;
        }
        else if (menuName == "leddbg")
        {
            animType = ANIMATION_TYPE_POINT_CONTROL;
        }

        else
        {
            return false;
        }

        setAnimation(animType);

        Serial.printf("Set LED State:%d  %s \n", animType, getAnimationName(animType));
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
    int ledIndex = (index % numLEDS + numLEDS) % numLEDS;
    if (ledIndex < 0 || ledIndex >= numLEDS)
    {
        Serial.printf("Invalid LED index %d\n", ledIndex);
        return;
    }
    if (invertLEDs)
        leds[numLEDS - ledIndex - 1] = CRGB(color.r, color.g, color.b);

    else
    {

        leds[ledIndex] = CRGB(color.r, color.g, color.b);
        // Serial.printf("Set Pixel %d %d %d %d\n", index, color.r, color.g, color.b);
    }
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
    bool invert = getBool(PARAM_INVERT);
    if (invert)
        leds[numLEDS - index - 1] = CRGB(0, 0, 0);
    else
        leds[index] = CRGB(0, 0, 0);
}

void StripState::respondToParameterMessage(parameter_message parameter)
{
    ParameterManager::respondToParameterMessage(parameter);

    if (parameter.paramID == PARAM_BEAT)
    {
        beatSize = parameter.value;
    }
    if (ledState == LED_STATE_POINT_CONTROL && parameter.paramID == PARAM_CURRENT_LED)
    {
        // Serial.printf("point control strip %s   %d \n", getName().c_str(), parameter.value);
    }

    if (ledState == LED_STATE_SINGLE_ANIMATION && parameter.paramID == PARAM_ANIMATION_TYPE)
    {
        animations.clear();
        addAnimation((ANIMATION_TYPE)parameter.value);
        currentAnimation = 0;
    }
    else if (ledState == LED_STATE_MULTI_ANIMATION && parameter.paramID == PARAM_ANIMATION_TYPE)
    {
        addAnimation((ANIMATION_TYPE)parameter.value);
        currentAnimation = animations.size() - 1;
    }
    if (currentAnimation < animations.size())
    {
        animations[currentAnimation]->respondToParameterMessage(parameter);
    }
}

#endif