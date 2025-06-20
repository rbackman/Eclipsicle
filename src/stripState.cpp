
#ifdef USE_LEDS

#include "stripState.h"
#include "animations.h"
int val = 0;
int minr = 0;
int maxr = 0;
led tempColor;

StripState::StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX, bool invert) : ParameterManager(("Strip" + String(STRIP_INDEX + 1)).c_str(), {PARAM_BRIGHTNESS, PARAM_CURRENT_STRIP, PARAM_SEQUENCE, PARAM_INVERT, PARAM_HUE, PARAM_CURRENT_LED}), ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX), invertLEDs(invert)

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

    case LED_STATE_POINT_CONTROL:
    {

        int pointPosition = (int)getInt(PARAM_CURRENT_LED);

        int pointHue = getInt(PARAM_HUE);
        int pointBrightness = getInt(PARAM_BRIGHTNESS);

        // Serial.printf("point control strip %s   %d \n", getName().c_str(), pointPosition);

        colorFromHSV(tempColor, float(pointHue) / float(255), 1, float(pointBrightness) / float(255));
        setPixel(pointPosition, tempColor);
    }
    break;
    default:
        break;
    }
}
enum MatchType
{
    MATCH_TYPE_NONE,
    MATCH_TYPE_STARTS_WITH,
    MATCH_TYPE_ENDS_WITH,
    MATCH_TYPE_CONTAINS,
    MATCH_TYPE_EQUALS
};
bool listContainsString(const std::vector<String> &list, const String &str, MatchType matchType = MATCH_TYPE_EQUALS)
{
    for (const auto &item : list)
    {
        if (matchType == MATCH_TYPE_STARTS_WITH && item.startsWith(str))
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_ENDS_WITH && item.endsWith(str))
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_CONTAINS && item.indexOf(str) != -1)
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_EQUALS && item.equals(str))
        {
            return true;
        }
    }
    return false;
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
        String menuTree = command.substring(5);
        menuTree.trim();
        menuTree.toLowerCase();
        Serial.printf("Menu tree: %s\n", menuTree.c_str());
        auto menuTreeList = splitString(menuTree, '/');
        ANIMATION_TYPE animType = ANIMATION_TYPE_NONE;
        if (listContainsString(menuTreeList, "particles"))
        {

            animType = ANIMATION_TYPE_PARTICLES;
        }
        else if (listContainsString(menuTreeList, "rainbow"))
        {

            animType = ANIMATION_TYPE_RAINBOW;
        }
        else if (listContainsString(menuTreeList, "random", MATCH_TYPE_STARTS_WITH))
        {

            animType = ANIMATION_TYPE_RANDOM;
        }
        else if (listContainsString(menuTreeList, "slider"))
        {

            animType = ANIMATION_TYPE_SLIDER;
        }

        else if (listContainsString(menuTreeList, "double", MATCH_TYPE_STARTS_WITH))
        {
            animType = ANIMATION_TYPE_DOUBLE_RAINBOW;
        }
        else if (listContainsString(menuTreeList, "idle"))
        {
            animType = ANIMATION_TYPE_IDLE;
        }
        else if (listContainsString(menuTreeList, "rndpart", MATCH_TYPE_STARTS_WITH))
        {
            animType = ANIMATION_TYPE_RANDOM_PARTICLES;
        }

        else if (listContainsString(menuTreeList, "slider"))
        {
            animType = ANIMATION_TYPE_SLIDER;
        }

        else if (listContainsString(menuTreeList, "double", MATCH_TYPE_STARTS_WITH))
        {
            animType = ANIMATION_TYPE_DOUBLE_RAINBOW;
        }
        else if (listContainsString(menuTreeList, "idle"))
        {
            animType = ANIMATION_TYPE_IDLE;
            return true;
        }
        else if (listContainsString(menuTreeList, "point", MATCH_TYPE_STARTS_WITH))
        {
            ledState = LED_STATE_POINT_CONTROL;
            animations.clear();
            if (verbose)
            {
                Serial.printf("Set LED state to POINT_CONTROL\n");
            }
            return true;
        }

        else
        {
            Serial.printf("Unknown animation type %s\n", command.c_str());
            return false;
        }
        if (animType == animations[0]->getAnimationType())
        {

            return true;
        }
        setAnimation(animType);

        if (isVerbose())
        {
            Serial.printf("Set animation %s\n", getAnimationName(animType).c_str());
        }

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

bool StripState::respondToParameterMessage(parameter_message parameter)
{

    if (parameter.paramID == PARAM_BEAT)
    {
        beatSize = parameter.value;
    }

    if (ledState == LED_STATE_SINGLE_ANIMATION && parameter.paramID == PARAM_ANIMATION_TYPE)
    {
        animations.clear();
        setAnimation((ANIMATION_TYPE)parameter.value);
        currentAnimation = 0;
    }
    else if (ledState == LED_STATE_MULTI_ANIMATION && parameter.paramID == PARAM_ANIMATION_TYPE)
    {
        addAnimation((ANIMATION_TYPE)parameter.value);
        currentAnimation = animations.size() - 1;
    }
    if (currentAnimation < animations.size())
    {
        Serial.printf("sending parameter message to animation  %d %d  %d of %d\n", currentAnimation, parameter.paramID, parameter.value, animations.size());
        if (animations[currentAnimation]->respondToParameterMessage(parameter))
        {
            return true;
        }
    }

    bool stripTookParam = ParameterManager::respondToParameterMessage(parameter);
    if (stripTookParam)
    {
        Serial.printf("Strip %d took parameter %d %d\n", stripIndex, parameter.paramID, parameter.value);
        return true;
    }
    else
    {
        Serial.printf("Strip %d did not take parameter %d %d\n", stripIndex, parameter.paramID, parameter.value);
    }
    return false;
}

#endif