
#ifdef USE_LEDS

#include "stripState.h"
#include "animations.h"
#include <ArduinoJson.h>
#include <stdexcept>
#include <cmath>
#include <ctype.h>
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

std::vector<String> getAnimationNames()
{
    std::vector<String> names;
    for (const auto &pair : ANIMATION_TYPE_NAMES)
    {
        names.push_back(pair.second);
    }
    return names;
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
            // Compress by hue/value; saturation is assumed full.
            CHSV hsv = rgb2hsv_approximate(leds[i]);

            result += String(hsv.hue) + "," + String(hsv.val) + ":" + String(count) + ";";
            count = 1;
        }
    }
    return result;
}

StripState::StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX,
                       std::vector<Node3D> nodes)
    : ParameterManager(("Strip" + String(STRIP_INDEX + 1)).c_str(), {PARAM_CURRENT_STRIP, PARAM_SEQUENCE, PARAM_INVERT, PARAM_HUE, PARAM_CURRENT_LED, PARAM_BRIGHTNESS}),
      ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX), nodes(nodes)

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
            a["start"] = anim->getstart();
            a["end"] = anim->getend();
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

String StripState::getStripStateCompact()
{
    String out = "Parameters:\n";
    for (const auto &p : getIntParameters())
    {
        out += String(p.id) + ":" + String(p.value) + "\n";
    }
    for (const auto &p : getFloatParameters())
    {
        out += String(p.id) + ":" + String(p.value, 2) + "\n";
    }
    for (const auto &p : getBoolParameters())
    {
        out += String(p.id) + ":" + String(p.value ? "1" : "0") + "\n";
    }
    out += "Animations:\n";
    for (const auto &anim : animations)
    {
        out += anim->describeCompact();
        out += "\n";
    }
    return out;
}

std::unique_ptr<StripAnimation> makeAnimation(StripState *stripState, ANIMATION_TYPE animType, int start, int end, std::map<ParameterID, float> params)
{
    switch (animType)
    {
    case ANIMATION_TYPE_PARTICLES:
        return std::make_unique<ParticleAnimation>(stripState, false, start, end, params);
    case ANIMATION_TYPE_RAINBOW:
        return std::make_unique<RainbowAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_DOUBLE_RAINBOW:
        return std::make_unique<DoubleRainbowAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_SLIDER:
        return std::make_unique<SliderAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_RANDOM:
        return std::make_unique<RandomAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_BRICKS:
        return std::make_unique<FallingBricksAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_NEBULA:
        return std::make_unique<NebulaAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_RANDOM_PARTICLES:
        return std::make_unique<ParticleAnimation>(stripState, true, start, end, params);
    case ANIMATION_TYPE_SINGLE_COLOR:
        return std::make_unique<SingleColorAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_SPHERE:
        return std::make_unique<SphereAnimation>(stripState, start, end, params);
    case ANIMATION_TYPE_PLANE:
        return std::make_unique<PlaneAnimation>(stripState, start, end, params);
    default:
        throw std::invalid_argument("Unknown animation type");
    }
}
void StripState::addAnimation(ANIMATION_TYPE anim, int start, int end, std::map<ParameterID, float> params)
{

    StripAnimation *currentAnim = nullptr;

    if (start < 0)
    {
        start = 0;
    }
    if (end < 0 || end >= numLEDS)
    {
        end = numLEDS - 1;
    }
    auto animation = makeAnimation(this, anim, start, end, params);
    if (animation == nullptr)
    {
        Serial.println("Failed to create animation");
        return;
    }

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

        int pointPosition = getInt(PARAM_CURRENT_LED);
        int brightness = getInt(PARAM_BRIGHTNESS);
        int pointHue = getInt(PARAM_HUE);
        int currentStrip = getInt(PARAM_CURRENT_STRIP);
        // point control uses full brightness; colour conversion expects value in
        // the range 0-1
        float value = (float)brightness / 255.0f;
        float hue = (float)pointHue / 360.0f;
        colorFromHSV(tempColor, hue, 1.0f, value);

        if (pointPosition >= 0 && pointPosition < numLEDS)
        {
            setPixel(pointPosition, tempColor);
        }
        else
        {
            // Serial.printf("Point control LED %d out of range for strip %d\n", pointPosition, stripIndex);
        }
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
            Serial.printf("\nsim:%d:%s\n", stripIndex, compressed.c_str());
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

static void skipSpaces(const String &expr, int &pos)
{
    while (pos < expr.length() && isspace(static_cast<unsigned char>(expr[pos])))
    {
        pos++;
    }
}

static float parseExpression(const String &expr, int &pos, const std::map<String, float> &vars);
static float parseTerm(const String &expr, int &pos, const std::map<String, float> &vars);
static float parseFactor(const String &expr, int &pos, const std::map<String, float> &vars);

static float parseFactor(const String &expr, int &pos, const std::map<String, float> &vars)
{
    skipSpaces(expr, pos);
    bool neg = false;
    if (pos < expr.length() && expr[pos] == '-')
    {
        neg = true;
        pos++;
    }
    skipSpaces(expr, pos);
    if (pos < expr.length() && expr[pos] == '(')
    {
        pos++;
        float val = parseExpression(expr, pos, vars);
        skipSpaces(expr, pos);
        if (pos < expr.length() && expr[pos] == ')')
        {
            pos++;
        }
        return neg ? -val : val;
    }

    int start = pos;
    while (pos < expr.length() && (isalnum(expr[pos]) || expr[pos] == '_' || expr[pos] == '.'))
    {
        pos++;
    }
    String token = expr.substring(start, pos);
    float val = 0.0f;
    if (token.length() == 0)
    {
        return 0.0f;
    }
    if ((token[0] >= '0' && token[0] <= '9') || token[0] == '.' || (token[0] == '-' && token.length() > 1))
    {
        val = token.toFloat();
    }
    else
    {
        auto it = vars.find(token);
        if (it != vars.end())
        {
            val = it->second;
        }
    }
    if (neg)
    {
        val = -val;
    }
    return val;
}

static float parseTerm(const String &expr, int &pos, const std::map<String, float> &vars)
{
    float val = parseFactor(expr, pos, vars);
    while (true)
    {
        skipSpaces(expr, pos);
        if (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/'))
        {
            char op = expr[pos++];
            float rhs = parseFactor(expr, pos, vars);
            if (op == '*')
            {
                val *= rhs;
            }
            else
            {
                val /= rhs;
            }
        }
        else
        {
            break;
        }
    }
    return val;
}

static float parseExpression(const String &expr, int &pos, const std::map<String, float> &vars)
{
    float val = parseTerm(expr, pos, vars);
    while (true)
    {
        skipSpaces(expr, pos);
        if (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-'))
        {
            char op = expr[pos++];
            float rhs = parseTerm(expr, pos, vars);
            if (op == '+')
            {
                val += rhs;
            }
            else
            {
                val -= rhs;
            }
        }
        else
        {
            break;
        }
    }
    return val;
}

static float evaluateExpression(const String &expr, const std::map<String, float> &vars)
{
    int pos = 0;
    return parseExpression(expr, pos, vars);
}

bool StripState::parseAnimationScript(String script)
{
    script.replace('|', '\n');
    std::vector<String> lines = splitString(script, '\n');
    bool inParams = false;
    bool inAnims = false;
    bool inVars = false;
    String configFile;
    std::map<ParameterID, float> paramOverrides;
    std::map<String, float> variables;
    struct AnimLine
    {
        ANIMATION_TYPE type;
        int start;
        int end;
        std::map<ParameterID, float> params;
    };
    std::vector<AnimLine> anims;

    for (auto &line : lines)
    {
        line.trim();
        if (line.length() == 0 || line.startsWith("#"))
            continue;
        if (line.startsWith("ConfigFile:"))
        {
            configFile = line.substring(String("ConfigFile:").length());
            configFile.trim();
            continue;
        }
        if (line.equalsIgnoreCase("Variables:") || line.equalsIgnoreCase("VARIABLES:"))
        {
            if (isVerbose())
            {
                Serial.printf("Entering variables section\n");
            }
            inVars = true;
            inParams = false;
            inAnims = false;
            continue;
        }
        if (line.equalsIgnoreCase("Parameters:") || line.equalsIgnoreCase("PARAMETERS:"))
        {
            if (isVerbose())
            {
                Serial.printf("Entering parameters section\n");
            }
            inParams = true;
            inAnims = false;
            inVars = false;
            continue;
        }
        if (line.equalsIgnoreCase("Animations:") || line.equalsIgnoreCase("ANIMATIONS:"))
        {
            if (isVerbose())
            {
                Serial.printf("Entering animations section\n");
            }
            inAnims = true;
            inParams = false;
            inVars = false;
            continue;
        }
        if (inVars)
        {
            int colon = line.indexOf(':');
            if (colon == -1)
                continue;
            String key = line.substring(0, colon);
            String val = line.substring(colon + 1);
            key.trim();
            val.trim();
            float v = evaluateExpression(val, variables);
            variables[key] = v;
            if (isVerbose())
            {
                Serial.printf("Variable %s = %f\n", key.c_str(), v);
            }
            continue;
        }
        if (inParams)
        {
            int colon = line.indexOf(':');
            if (colon == -1)
                continue;
            String key = line.substring(0, colon);
            String val = line.substring(colon + 1);
            key.trim();
            val.trim();
            if (variables.find(val) != variables.end())
            {
                val = String(variables[val]);
            }
            ParameterID pid = PARAM_UNKNOWN;
            bool numeric = true;
            for (int i = 0; i < key.length(); ++i)
            {
                if (!isDigit(key[i]))
                {
                    numeric = false;
                    break;
                }
            }
            if (numeric)
            {
                pid = (ParameterID)key.toInt();
            }
            else
            {
                key.toUpperCase();
                String full = "PARAM_" + key;
                pid = getParameterID(full.c_str());
            }
            if (pid != PARAM_UNKNOWN)
            {
                paramOverrides[pid] = val.toFloat();
                if (isVerbose())
                {
                    Serial.printf("Parameter override: %d = %s\n", pid, val.c_str());
                }
            }
            else
            {
                Serial.printf("Unknown parameter in script: %s\n", key.c_str());
            }
            continue;
        }
        if (inAnims)
        {
            auto tokens = splitString(line, ' ');
            if (tokens.size() == 0)
                continue;
            String animName = tokens[0];
            ANIMATION_TYPE type = getAnimationTypeFromName(animName);
            if (type == ANIMATION_TYPE_NONE)
            {
                Serial.printf("Unknown animation type in script: %s out of: ", animName.c_str());
                for (const auto &name : getAnimationNames())
                {
                    Serial.printf(" %s ", name.c_str());
                }
                Serial.print(" \n");
                continue;
            }
            if (isVerbose())
            {
                Serial.printf(" animation type:%s \n", animName.c_str());
            }
            int start = 0;
            int end = getNumLEDS() - 1;
            std::map<ParameterID, float> params;
            for (int i = 1; i < tokens.size(); ++i)
            {
                String t = tokens[i];
                int c = t.indexOf(':');
                if (c == -1)
                    continue;
                String k = t.substring(0, c);
                String v = t.substring(c + 1);
                k.trim();
                v.trim();
                if (variables.find(v) != variables.end())
                {
                    v = String(variables[v]);
                }
                if (k.equalsIgnoreCase("start"))
                {
                    if (v.equalsIgnoreCase("mid"))
                    {
                        start = getMidLed();
                    }
                    else if (v.startsWith("n"))
                    {
                        int idx = v.substring(1).toInt();
                        start = getNode(idx);
                    }
                    else
                    {
                        start = v.toInt();
                    }
                    if (isVerbose())
                    {
                        Serial.printf("\tstart = %d\n", start);
                    }
                }
                else if (k.equalsIgnoreCase("end"))
                {
                    if (v.equalsIgnoreCase("mid"))
                    {
                        end = getMidLed();
                    }
                    else if (v.startsWith("n"))
                    {
                        int idx = v.substring(1).toInt();
                        end = getNode(idx);
                    }
                    else
                    {
                        end = v.toInt();
                    }
                    if (isVerbose())
                    {
                        Serial.printf("\tend = %d\n", end);
                    }
                }
                else
                {
                    ParameterID pid = PARAM_UNKNOWN;
                    bool numeric = true;
                    for (int cidx = 0; cidx < k.length(); ++cidx)
                    {
                        if (!isDigit(k[cidx]))
                        {
                            numeric = false;
                            break;
                        }
                    }
                    if (numeric)
                    {
                        pid = (ParameterID)k.toInt();
                    }
                    else
                    {
                        k.toUpperCase();
                        String full = "PARAM_" + k;
                        pid = getParameterID(full.c_str());
                    }
                    if (isBoolParameter(pid))
                    {
                        if (v.equalsIgnoreCase("true") || v.equalsIgnoreCase("1"))
                        {
                            v = "1";
                        }
                        else if (v.equalsIgnoreCase("false") || v.equalsIgnoreCase("0"))
                        {
                            v = "0";
                        }
                        else
                        {
                            Serial.printf("Invalid boolean value for parameter %s: %s\n", full.c_str(), v.c_str());
                            continue;
                        }
                    }
                    else if (isIntParameter(pid))
                    {
                    }
                    else if (isFloatParameter(pid))
                    {
                    }
                    else
                    {
                        Serial.printf("Unknown parameter in script: %s\n", k.c_str());
                    }
                    if (pid != PARAM_UNKNOWN)
                    {
                        params[pid] = v.toFloat();
                    }
                }
            }
            anims.push_back({type, start, end, params});
        }
    }

    ledState = LED_STATE_MULTI_ANIMATION;
    animations.clear();

    if (configFile.length() > 0 && isVerbose())
    {
        Serial.printf("\nConfigFile\n %s\n", configFile.c_str());
    }

    for (const auto &a : anims)
    {
        addAnimation(a.type, a.start, a.end, a.params);
    }
    return true;
}
void StripState::replaceAnimation(int index, ANIMATION_TYPE animType, std::map<ParameterID, float> params)
{
    if (index < 0 || index >= animations.size())
    {
        Serial.printf("Invalid animation index %d\n", index);
        return;
    }
    int start = animations[index]->getstart();
    int end = animations[index]->getend();
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
    if (command.startsWith("script:"))
    {
        String script = command.substring(String("script:").length());
        return parseAnimationScript(script);
    }
    if (command.startsWith("get_nodes"))
    {
        String nodeString = "nodes:";
        nodeString += String(stripIndex) + ":" + String(numLEDS) + ":" + String(nodes.size()) + ":";
        for (const auto &node : nodes)
        {
            nodeString += String(node.index) + "," + String(node.x) + "," + String(node.y) + "," + String(node.z) + ":";
        }
        Serial.println(nodeString);
        return true;
    }
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

        if (animName == "POINT_CONTROL")
        {
            ledState = LED_STATE_POINT_CONTROL;
            if (isVerbose())
            {
                Serial.println("Set LED state to POINT_CONTROL");
            }
            return true;
        }
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        if (animType == ANIMATION_TYPE_NONE)
        {
            Serial.printf("Unknown animation type %s\n", animName.c_str());
            return false;
        }

        if (animparts.size() == 4)
        {
            int start = animparts[2].toInt();
            int end = animparts[3].toInt();
            setAnimation(animType, start, end);
            if (isVerbose())
            {
                Serial.printf("Set animation %s from %d to %d \n", getAnimationName(animType).c_str(), start, end);
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
                    Serial.printf("Replaced animation at index %d with %s %d %d\n", i, getAnimationName(anim->getAnimationType()).c_str(), anim->getstart(), anim->getend());
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
        int start = 0;
        int end = getNumLEDS() - 1;
        if (animparts.size() == 4)
        {
            start = animparts[2].toInt();
            end = animparts[3].toInt();
        }
        addAnimation(animType, start, end);
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

    bool stripTookParam = ParameterManager::respondToParameterMessage(parameter);
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

    return stripTookParam;
}

Node3D StripState::getLEDPosition(int ledIndex)
{
    if (nodes.empty())
    {
        return {ledIndex, float(ledIndex), 0.0f, 0.0f};
    }
    if (ledIndex <= nodes.front().index)
    {
        return {ledIndex, nodes.front().x, nodes.front().y, nodes.front().z};
    }
    if (ledIndex >= nodes.back().index)
    {
        return {ledIndex, nodes.back().x, nodes.back().y, nodes.back().z};
    }
    for (size_t i = 0; i < nodes.size() - 1; ++i)
    {
        const auto &a = nodes[i];
        const auto &b = nodes[i + 1];
        if (ledIndex >= a.index && ledIndex <= b.index)
        {
            float t = float(ledIndex - a.index) / float(b.index - a.index);
            float x = a.x + (b.x - a.x) * t;
            float y = a.y + (b.y - a.y) * t;
            float z = a.z + (b.z - a.z) * t;
            return {ledIndex, x, y, z};
        }
    }
    return {ledIndex, 0.0f, 0.0f, 0.0f};
}

#endif
