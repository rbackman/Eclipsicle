
#ifdef USE_LEDS

#include "stripState.h"
#include "animations.h"
#include <ArduinoJson.h>
int val = 0;
int minr = 0;
int maxr = 0;
led tempColor;

ANIMATION_TYPE getAnimationTypeFromName(const String &name)
{
    for (const auto &pair : ANIMATION_TYPE_NAMES)
    {
        if (pair.second.equalsIgnoreCase(name))
        {
            return pair.first;
        }
    }
    return ANIMATION_TYPE_NONE;
}

String rleCompresssCRGB(const CRGB *leds, int numLEDS)
{
    String result;
    int count = 1;

    for (int i = 0; i < numLEDS; i++)
    {
        if (i < numLEDS - 1 && leds[i] == leds[i + 1])
        {
            count++;
        }
        else
        {
            // send just the hue value
            int hue = 0;
            CHSV hsv = rgb2hsv_approximate(leds[i]);

            result += String(hsv.hue) + "," + String(hsv.value) + ":" + String(count) + ";";
            count = 1;
        }
    }
    return result;
}

StripState::StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX) : ParameterManager(("Strip" + String(STRIP_INDEX + 1)).c_str(), {PARAM_CURRENT_STRIP, PARAM_SEQUENCE, PARAM_INVERT, PARAM_HUE, PARAM_CURRENT_LED}), ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX)

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

String StripState::getStripState(bool verbose)
{
    if (!verbose)
    {
        return getLedStateName(ledState);
    }
    String result = "strip" + String(stripIndex + 1) + " state:" + getLedStateName(ledState);
    for (const auto &anim : animations)
    {
        result += " " + anim->describe();
        result += ";";
    }
    return result;
}

String StripState::getStripStateJson(bool verbose)
{
    JsonDocument doc;
    doc["type"] = "stripState";
    doc["strip"] = stripIndex + 1;
    doc["state"] = getLedStateName(ledState);
    if (verbose)
    {
        JsonArray arr = doc["animations"].to<JsonArray>();
        for (const auto &anim : animations)
        {
            JsonObject a = arr.add<JsonObject>();
            a["type"] = getAnimationName(anim->getAnimationType()).c_str();
            a["start"] = anim->getStartLED();
            a["end"] = anim->getEndLED();
            JsonObject params = a["params"].to<JsonObject>();
            for (const auto &p : anim->getIntParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
            for (const auto &p : anim->getFloatParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
            for (const auto &p : anim->getBoolParameters())
            {
                params[getParameterName(p.id).c_str()] = p.value;
            }
        }
    }
    String output;
    serializeJson(doc, output);
    return output;
}

std::unique_ptr<StripAnimation> makeAnimation(StripState *stripState, ANIMATION_TYPE animType, int startLED, int endLED, std::map<ParameterID, float> params)
{
    switch (animType)
    {
    case ANIMATION_TYPE_PARTICLES:
        return std::make_unique<ParticleAnimation>(stripState, false, startLED, endLED);
    case ANIMATION_TYPE_RAINBOW:
        return std::make_unique<RainbowAnimation>(stripState, startLED, endLED);
    case ANIMATION_TYPE_DOUBLE_RAINBOW:
        return std::make_unique<DoubleRainbowAnimation>(stripState, startLED, endLED);
    case ANIMATION_TYPE_SLIDER:
        return std::make_unique<SliderAnimation>(stripState, startLED, endLED);
    case ANIMATION_TYPE_RANDOM:
        return std::make_unique<RandomAnimation>(stripState, startLED, endLED);
    case ANIMATION_TYPE_RANDOM_PARTICLES:
        return std::make_unique<ParticleAnimation>(stripState, true, startLED, endLED);
    default:
        throw std::invalid_argument("Unknown animation type");
    }
}
void StripState::addAnimation(ANIMATION_TYPE anim, int startLED, int endLED, std::map<ParameterID, float> params)
{

    StripAnimation *currentAnim = nullptr;

    if (startLED < 0)
    {
        startLED = 0;
    }
    if (endLED < 0 || endLED >= numLEDS)
    {
        endLED = numLEDS - 1;
    }
    auto animation = makeAnimation(this, anim, startLED, endLED, params);
    if (animation == nullptr)
    {
        Serial.println("Failed to create animation");
        return;
    }
    animation.get()->setParameters(params);
    animations.emplace_back(std::move(animation));
}
void StripState::update()
{
    counter++;
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

        for (int i = 0; i < animations.size(); i++)
        {
            animations[i].get()->update();
        }
    }

    break;

    case LED_STATE_POINT_CONTROL:
    {

        int pointPosition = (int)getInt(PARAM_CURRENT_LED);

        int pointHue = getInt(PARAM_HUE);

        // Serial.printf("point control strip %s   %d \n", getName().c_str(), pointPosition);

        colorFromHSV(tempColor, float(pointHue) / float(255), 1, 255);
        setPixel(pointPosition, tempColor);
    }
    break;
    default:
        break;
    }

    if (simulateCount > 0)
    {
        if (counter % simulateCount == 0)
        {
            String compressed = rleCompresssCRGB(leds, numLEDS);
            Serial.printf("\nsim:%s\n", compressed.c_str());
        }
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
void StripState::replaceAnimation(int index, ANIMATION_TYPE animType, std::map<ParameterID, float> params)
{
    if (index < 0 || index >= animations.size())
    {
        Serial.printf("Invalid animation index %d\n", index);
        return;
    }
    int start = animations[index]->getStartLED();
    int end = animations[index]->getEndLED();
    animations[index] = makeAnimation(this, animType, start, end, params);
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
    // if (command.startsWith("menu:"))
    // {
    //     String menuTree = command.substring(5);
    //     menuTree.trim();
    //     menuTree.toLowerCase();
    //     Serial.printf("Menu tree: %s\n", menuTree.c_str());
    //     auto menuTreeList = splitString(menuTree, '/');
    //     ANIMATION_TYPE animType = ANIMATION_TYPE_NONE;

    //     if (listContainsString(menuTreeList, "patterns"))
    //     {
    //         if (listContainsString(menuTreeList, "particles"))
    //         {
    //             animType = ANIMATION_TYPE_PARTICLES;
    //         }
    //         else if (listContainsString(menuTreeList, "rainbow"))
    //         {
    //             animType = ANIMATION_TYPE_RAINBOW;
    //         }
    //         else if (listContainsString(menuTreeList, "random", MATCH_TYPE_STARTS_WITH))
    //         {
    //             animType = ANIMATION_TYPE_RANDOM;
    //         }
    //         else if (listContainsString(menuTreeList, "slider"))
    //         {
    //             animType = ANIMATION_TYPE_SLIDER;
    //         }

    //         else if (listContainsString(menuTreeList, "double", MATCH_TYPE_STARTS_WITH))
    //         {
    //             animType = ANIMATION_TYPE_DOUBLE_RAINBOW;
    //         }
    //         else if (listContainsString(menuTreeList, "idle"))
    //         {
    //             animType = ANIMATION_TYPE_IDLE;
    //         }
    //         else if (listContainsString(menuTreeList, "rndpart", MATCH_TYPE_STARTS_WITH))
    //         {
    //             animType = ANIMATION_TYPE_RANDOM_PARTICLES;
    //         }

    //         else if (listContainsString(menuTreeList, "slider"))
    //         {
    //             animType = ANIMATION_TYPE_SLIDER;
    //         }

    //         else if (listContainsString(menuTreeList, "double", MATCH_TYPE_STARTS_WITH))
    //         {
    //             animType = ANIMATION_TYPE_DOUBLE_RAINBOW;
    //         }
    //         else if (listContainsString(menuTreeList, "idle"))
    //         {
    //             animType = ANIMATION_TYPE_IDLE;
    //             return true;
    //         }
    //         else if (listContainsString(menuTreeList, "point", MATCH_TYPE_STARTS_WITH))
    //         {
    //             ledState = LED_STATE_POINT_CONTROL;
    //             animations.clear();
    //             if (verbose)
    //             {
    //                 Serial.printf("Set LED state to POINT_CONTROL\n");
    //             }
    //             return true;
    //         }

    //         else
    //         {
    //             Serial.printf("Unknown animation type %s\n", command.c_str());
    //             return false;
    //         }
    //         if (animType == animations[0]->getAnimationType())
    //         {
    //             return true;
    //         }
    //         setAnimation(animType);
    //     }
    //     if (isVerbose())
    //     {
    //         Serial.printf("Set animation %s\n", getAnimationName(animType).c_str());
    //     }

    //     return true;
    // }
    if (command.startsWith("simulate:"))
    {
        auto simParts = splitString(command, ':');
        if (simParts.size() != 2)
        {
            Serial.printf("Invalid simulate command %s\n", command.c_str());
            return false;
        }
        int simulate = simParts[1].toInt();
        setSimulate(simulate);
        if (isVerbose())
        {
            Serial.printf("Set simulate to %d\n", simulate);
        }
        return true;
    }
    if (command.startsWith("setanimation:"))
    {
        ledState = LED_STATE_SINGLE_ANIMATION;
        auto animparts = splitString(command, ':');
        auto animName = animparts[1];
        animName.trim();

        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        if (animType == ANIMATION_TYPE_NONE)
        {
            Serial.printf("Unknown animation type %s\n", animName.c_str());
            return false;
        }

        if (animparts.size() == 4)
        {
            int startLED = animparts[2].toInt();
            int endLED = animparts[3].toInt();
            setAnimation(animType, startLED, endLED);
            if (isVerbose())
            {
                Serial.printf("Set animation %s from %d to %d \n", getAnimationName(animType).c_str(), startLED, endLED);
            }
        }
        else if (animparts.size() == 2)
        {

            setAnimation(animType, 0, getNumLEDS() - 1);
            if (isVerbose())
            {
                Serial.printf("Set animation %s\n", getAnimationName(animType).c_str());
            }
        }

        return true;
    }

    if (command.startsWith("replaceanimation:"))
    {

        auto animparts = splitString(command, ':');
        if (animparts.size() < 3)
        {
            Serial.printf("Invalid replaceanimation command %s\n", command.c_str());
            return false;
        }
        auto animName = animparts[2];
        animName.trim();
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        auto whichPart = animparts[1];
        whichPart.trim();
        if (whichPart.equalsIgnoreCase("all"))
        {
            if (isVerbose())
                Serial.printf("Replacing all %d animations with %s\n", animations.size(), animName.c_str());

            for (int i = 0; i < animations.size(); i++)
            {
                replaceAnimation(i, animType);
            }
            if (isVerbose())
            {
                for (int i = 0; i < animations.size(); i++)
                {
                    auto &anim = animations[i];
                    Serial.printf("Replaced animation at index %d with %s %d %d\n", i, getAnimationName(anim->getAnimationType()).c_str(), anim->getStartLED(), anim->getEndLED());
                }
            }
            return true;
        }
        else
        {

            int index = whichPart.toInt();
            if (index < 0 || index >= animations.size())
            {
                Serial.printf("Invalid animation index %d\n", index);
                return false;
            }
            if (animType == ANIMATION_TYPE_NONE)
            {
                Serial.printf("Unknown animation type %s\n", animName.c_str());
                return false;
            }
            replaceAnimation(index, animType);
            if (isVerbose())
            {
                Serial.printf("Replaced animation at index %d with %s\n", index, getAnimationName(animType).c_str());
            }
            return true;
        }
    }
    if (command.startsWith("addanimation:"))
    {
        ledState = LED_STATE_MULTI_ANIMATION;
        auto animparts = splitString(command, ':');
        if (animparts.size() < 2)
        {
            Serial.printf("Invalid addanimation command %s\n", command.c_str());
            return false;
        }
        auto animName = animparts[1];
        animName.trim();
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        if (animType == ANIMATION_TYPE_NONE)
        {
            Serial.printf("Unknown animation type %s\n", animName.c_str());
            return false;
        }
        int startLED = 0;
        int endLED = getNumLEDS() - 1;
        if (animparts.size() == 4)
        {
            startLED = animparts[2].toInt();
            endLED = animparts[3].toInt();
        }
        addAnimation(animType, startLED, endLED);
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
    leds[ledIndex] = CRGB(color.r, color.g, color.b);
}

void StripState::blendPixel(int index, led color)
{
    int ledIndex = (index % numLEDS + numLEDS) % numLEDS;
    if (ledIndex < 0 || ledIndex >= numLEDS)
    {
        Serial.printf("Invalid LED index %d\n", ledIndex);
        return;
    }
    int r = leds[ledIndex].r + color.r;
    int g = leds[ledIndex].g + color.g;
    int b = leds[ledIndex].b + color.b;

    if (r > 255)
        r = 255;
    if (g > 255)
        g = 255;
    if (b > 255)
        b = 255;

    leds[ledIndex] = CRGB(r, g, b);
}
void StripState::setPixel(int index, int r, int g, int b)
{

    leds[index] = CRGB(r, g, b);
}
void StripState::clearPixel(int index)
{
    bool invert = getBool(PARAM_INVERT);

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
        bool animationTookParam = false;
        for (int i = 0; i < animations.size(); i++)
        {
            if (animations[i]->respondToParameterMessage(parameter))
            {
                animationTookParam = true;
            }
        }
        if (animationTookParam)
        {
            return true;
        }
    }

    bool stripTookParam = ParameterManager::respondToParameterMessage(parameter);

    return false;
}

#endif