
#ifdef USE_LEDS

#include "stripState.h"
#include "animations.h"

#include <stdexcept>
#include <cmath>
#include <cctype>
#include <sstream>
#include <memory>
#include "log.h"
#include "string_utils.h"

#ifdef LED_BASIC
static constexpr size_t BASIC_CACHE_LIMIT = 3;
#endif

int val = 0;
int minr = 0;
int maxr = 0;
led tempColor;

ANIMATION_TYPE getAnimationTypeFromName(const std::string &name)
{
    for (const auto &pair : ANIMATION_TYPE_NAMES)
    {
        if (equalsIgnoreCase(pair.second, name))
        {
            return pair.first;
        }
    }
    return ANIMATION_TYPE_NONE;
}

std::string rleCompresssCRGB(const CRGB *leds, int numLEDS)
{
    std::string result;
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

            result += std::to_string(hsv.hue) + "," + std::to_string(hsv.val) + ":" + std::to_string(count) + ";";
            count = 1;
        }
    }
    return result;
}

StripState::StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX,
                       std::vector<Node3D> nodes)
    : ParameterManager("Strip" + std::to_string(STRIP_INDEX + 1), {PARAM_CURRENT_STRIP, PARAM_SEQUENCE, PARAM_INVERT, PARAM_HUE, PARAM_CURRENT_LED, PARAM_BRIGHTNESS}),
      ledState(state), numLEDS(numLEDS), stripIndex(STRIP_INDEX), nodes(nodes)

{
    leds = new CRGB[numLEDS];
}

void StripState::setAnimationFromName(std::string animationName)
{
    ANIMATION_TYPE animType = getAnimationTypeFromName(animationName);
    if (animType == ANIMATION_TYPE_NONE)
    {

        return;
    }
    setAnimation(animType);
}
void StripState::toggleMode()
{
    ledState = (LED_STATE)(((int)ledState + 1) % LED_STATE_COUNT);
    LOG_PRINTLN("LED State: ");
    LOG_PRINTLN(getLedStateName(ledState).c_str());
    this->clearPixels();
}

std::string StripState::getStripState(bool verbose)
{
    if (!verbose)
    {
        return getLedStateName(ledState);
    }
    std::string result = "strip" + std::to_string(stripIndex + 1) + " state:" + getLedStateName(ledState);
    for (const auto &anim : animations)
    {
        result += " " + anim->describe();
        result += ";";
    }
    return result;
}

std::string StripState::getStripStateCompact()
{
    std::string out = "p:\n";
    for (const auto &p : getIntParameters())
    {
        out += std::to_string(p.id) + ":" + std::to_string(p.value) + "\n";
    }
    for (const auto &p : getFloatParameters())
    {
        out += std::to_string(p.id) + ":" + std::to_string(p.value) + "\n";
    }
    for (const auto &p : getBoolParameters())
    {
        out += std::to_string(p.id) + ":" + (p.value ? "1" : "0") + "\n";
    }
    out += "a:\n";
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
        return std::unique_ptr<ParticleAnimation>(new ParticleAnimation(stripState, false, start, end, params));
    case ANIMATION_TYPE_RAINBOW:
        return std::unique_ptr<RainbowAnimation>(new RainbowAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_DOUBLE_RAINBOW:
        return std::unique_ptr<DoubleRainbowAnimation>(new DoubleRainbowAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_SLIDER:
        return std::unique_ptr<SliderAnimation>(new SliderAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_RANDOM:
        return std::unique_ptr<RandomAnimation>(new RandomAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_BRICKS:
        return std::unique_ptr<FallingBricksAnimation>(new FallingBricksAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_NEBULA:
        return std::unique_ptr<NebulaAnimation>(new NebulaAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_RANDOM_PARTICLES:
        return std::unique_ptr<ParticleAnimation>(new ParticleAnimation(stripState, true, start, end, params));
    case ANIMATION_TYPE_SINGLE_COLOR:
        return std::unique_ptr<SingleColorAnimation>(new SingleColorAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_SPHERE:
        return std::unique_ptr<SphereAnimation>(new SphereAnimation(stripState, start, end, params));
    case ANIMATION_TYPE_PLANE:
        return std::unique_ptr<PlaneAnimation>(new PlaneAnimation(stripState, start, end, params));
#ifdef LED_BASIC
    case ANIMATION_TYPE_BASIC_SCRIPT:
        return std::unique_ptr<BasicScriptAnimation>(
            new BasicScriptAnimation(stripState, start, end, stripState->getBasicScript(), params));
#endif
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
        LOG_PRINTLN("Failed to create animation");
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
            // LOG_PRINTF("Point control LED %d out of range for strip %d\n", pointPosition, stripIndex);
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
            std::string compressed = rleCompresssCRGB(leds, numLEDS);
            LOG_PRINTF("\nsim:%d:%s\n", stripIndex, compressed.c_str());
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
bool listContainsString(const std::vector<std::string> &list, const std::string &str, MatchType matchType = MATCH_TYPE_EQUALS)
{
    for (const auto &item : list)
    {
        if (matchType == MATCH_TYPE_STARTS_WITH && startsWith(item, str))
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_ENDS_WITH && endsWith(item, str))
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_CONTAINS && indexOf(item, str) != -1)
        {
            return true;
        }
        else if (matchType == MATCH_TYPE_EQUALS && item == str)
        {
            return true;
        }
    }
    return false;
}

static void skipSpaces(const std::string &expr, int &pos)
{
    while (pos < expr.length() && isspace(static_cast<unsigned char>(expr[pos])))
    {
        pos++;
    }
}

static float parseExpression(const std::string &expr, int &pos, const std::map<std::string, float> &vars);
static float parseTerm(const std::string &expr, int &pos, const std::map<std::string, float> &vars);
static float parseFactor(const std::string &expr, int &pos, const std::map<std::string, float> &vars);

static float parseFactor(const std::string &expr, int &pos, const std::map<std::string, float> &vars)
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
    std::string token = substring(expr, start, pos);
    float val = 0.0f;
    if (token.length() == 0)
    {
        return 0.0f;
    }
    if ((token[0] >= '0' && token[0] <= '9') || token[0] == '.' || (token[0] == '-' && token.length() > 1))
    {
        val = toFloat(token);
    }
    else
    {
        auto it = vars.find(token);
        if (it != vars.end())
        {
            val = it->second;
        }
        else
        {
            LOG_PRINTF("Unknown variable or constant: %s\n", token.c_str());
        }
    }
    if (neg)
    {
        val = -val;
    }
    return val;
}

static float parseTerm(const std::string &expr, int &pos, const std::map<std::string, float> &vars)
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

static float parseExpression(const std::string &expr, int &pos, const std::map<std::string, float> &vars)
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

static float evaluateExpression(const std::string &expr, const std::map<std::string, float> &vars)
{
    int pos = 0;
    return parseExpression(expr, pos, vars);
}

bool StripState::parseAnimationScript(std::string script)
{

    replace(script, "|", "\n");
    std::vector<std::string> lines = splitString(script, '\n');
    bool inParams = false;
    bool inAnims = false;
    bool inVars = false;
    std::string configFile;
    std::map<ParameterID, float> paramOverrides;
    std::map<std::string, float> variables;
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
        trim(line);
        if (line.length() == 0 || startsWith(line, "#"))
            continue;
        if (startsWith(line, "ConfigFile:"))
        {
            configFile = substring(line, 11);
            trim(configFile);
            continue;
        }
        if (equalsIgnoreCase(line, "Variables:") || equalsIgnoreCase(line, "VARIABLES:") || equalsIgnoreCase(line, "v:"))
        {
            if (isVerbose())
            {
                LOG_PRINTF("Entering variables section\n");
            }
            inVars = true;
            inParams = false;
            inAnims = false;
            continue;
        }
        if (equalsIgnoreCase(line, "p:") || equalsIgnoreCase(line, "PARAMETERS:") || equalsIgnoreCase(line, "p:"))
        {

            inParams = true;
            inAnims = false;
            inVars = false;
            continue;
        }
        if (equalsIgnoreCase(line, "a:") || equalsIgnoreCase(line, "ANIMATIONS:") || equalsIgnoreCase(line, "a:"))
        {

            inAnims = true;
            inParams = false;
            inVars = false;
            continue;
        }
        if (inVars)
        {
            int colon = indexOf(line, ':');
            if (colon == -1)
                continue;
            std::string key = substring(line, 0, colon);
            std::string val = substring(line, colon + 1);
            trim(key);
            trim(val);
            float v = evaluateExpression(val, variables);
            variables[key] = v;
            if (isVerbose())
            {
                LOG_PRINTF("Variable %s = %f\n", key.c_str(), v);
            }
            continue;
        }
        if (inParams)
        {
            int colon = indexOf(line, ':');
            if (colon == -1)
                continue;
            std::string key = substring(line, 0, colon);
            std::string val = substring(line, colon + 1);
            trim(key);
            trim(val);
            if (variables.find(val) != variables.end())
            {
                val = std::to_string(variables[val]);
            }
            ParameterID pid = PARAM_UNKNOWN;
            bool numeric = true;
            for (int i = 0; i < key.length(); ++i)
            {
                if (!std::isdigit(static_cast<unsigned char>(key[i])))
                {
                    numeric = false;
                    break;
                }
            }
            if (numeric)
            {
                pid = (ParameterID)toInt(key);
            }
            else
            {
                toUpperCase(key);
                std::string full = "PARAM_" + key;
                pid = getParameterID(full.c_str());
            }
            if (pid != PARAM_UNKNOWN)
            {
                paramOverrides[pid] = toFloat(val);
                if (isVerbose())
                {
                    LOG_PRINTF("Parameter override: %d = %s\n", pid, val.c_str());
                }
            }
            else
            {
                LOG_PRINTF("Unknown parameter in script: %s\n", key.c_str());
            }
            continue;
        }
        if (inAnims)
        {
            auto tokens = splitString(line, ' ');
            if (tokens.size() == 0)
                continue;
            std::string animName = tokens[0];
            //  if animName is an int turn it into an animation type
            ANIMATION_TYPE type = toInt(animName) ? (ANIMATION_TYPE)toInt(animName) : getAnimationTypeFromName(animName);
            if (type == ANIMATION_TYPE_NONE)
            {
                LOG_PRINTF("Unknown animation type in script: %s out of: ", animName.c_str());
                for (const auto &name : getAnimationNames())
                {
                    LOG_PRINTF(" %s ", name.c_str());
                }
                LOG_PRINT(" \n");
                continue;
            }
            if (isVerbose())
            {
                LOG_PRINTF(" animation type:%s \n", animName.c_str());
            }
            int start = 0;
            int end = getNumLEDS() - 1;
            std::map<ParameterID, float> params;
            for (int i = 1; i < tokens.size(); ++i)
            {
                std::string t = tokens[i];
                int c = indexOf(t, ':');
                if (c == -1)
                    continue;
                std::string k = substring(t, 0, c);
                std::string v = substring(t, c + 1);
                trim(k);
                trim(v);
                if (variables.find(v) != variables.end())
                {
                    v = std::to_string(variables[v]);
                }
                if (equalsIgnoreCase(k, "start"))
                {
                    if (equalsIgnoreCase(v, "mid"))
                    {
                        start = getMidLed();
                    }
                    else if (startsWith(v, "n"))
                    {
                        int idx = toInt(substring(v, 1));
                        start = getNode(idx);
                    }
                    else
                    {
                        start = toInt(v);
                    }
                    if (isVerbose())
                    {
                        LOG_PRINTF("\tstart = %d\n", start);
                    }
                }
                else if (equalsIgnoreCase(k, "end"))
                {
                    if (equalsIgnoreCase(v, "mid"))
                    {
                        end = getMidLed();
                    }
                    else if (startsWith(v, "n"))
                    {
                        int idx = toInt(substring(v, 1));
                        end = getNode(idx);
                    }
                    else
                    {
                        end = toInt(v);
                    }
                    if (isVerbose())
                    {
                        LOG_PRINTF("\tend = %d\n", end);
                    }
                }
                else
                {
                    ParameterID pid = PARAM_UNKNOWN;
                    bool numeric = true;
                    for (int cidx = 0; cidx < k.length(); ++cidx)
                    {
                        if (!std::isdigit(static_cast<unsigned char>(k[cidx])))
                        {
                            numeric = false;
                            break;
                        }
                    }
                    if (numeric)
                    {
                        pid = (ParameterID)toInt(k);
                    }
                    else
                    {
                        toUpperCase(k);
                        std::string full = "PARAM_" + k;
                        pid = getParameterID(full.c_str());
                    }
                    if (isBoolParameter(pid))
                    {
                        if (equalsIgnoreCase(v, "true") || equalsIgnoreCase(v, "1"))
                        {
                            v = "1";
                        }
                        else if (equalsIgnoreCase(v, "false") || equalsIgnoreCase(v, "0"))
                        {
                            v = "0";
                        }
                        else
                        {
                            LOG_PRINTF("Invalid boolean value for parameter %s: %s\n", k.c_str(), v.c_str());
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
                        LOG_PRINTF("Unknown parameter in script: %s\n", k.c_str());
                    }
                    if (pid != PARAM_UNKNOWN)
                    {
                        params[pid] = toFloat(v);
                    }
                }
            }
            anims.push_back({type, start, end, params});
        }
    }

    ledState = LED_STATE_MULTI_ANIMATION;
    animations.clear();

    for (const auto &a : anims)
    {
        addAnimation(a.type, a.start, a.end, a.params);
    }
    return true;
}

#ifdef LED_BASIC
std::shared_ptr<BasicLEDController> StripState::getCachedBasicProgram(const std::string &script, int start, int end)
{
    int length = (end < 0) ? numLEDS - start : end - start + 1;
    for (auto it = basicProgramCache.begin(); it != basicProgramCache.end(); ++it)
    {
        if (it->script == script && it->start == start && it->length == length)
        {
            auto controller = it->controller;
            basicProgramCache.splice(basicProgramCache.begin(), basicProgramCache, it);
            return controller;
        }
    }

    auto controller = std::make_shared<BasicLEDController>(leds + start, length);
    if (!controller->loadProgram(String(script.c_str())))
    {
        LOG_PRINTF("Failed to load BASIC program: %s\n", script.c_str());
        return nullptr;
    }
    controller->runSetup();
    basicProgramCache.push_front({script, start, length, controller});
    if (basicProgramCache.size() > BASIC_CACHE_LIMIT)
    {
        basicProgramCache.pop_back();
    }
    return controller;
}
#endif

bool StripState::parseBasicScript(std::string script)
{
#ifdef LED_BASIC
    replace(script, "|", "\n");
    basicScript = script;
    ledState = LED_STATE_SINGLE_ANIMATION;
    animations.clear();
    addAnimation(ANIMATION_TYPE_BASIC_SCRIPT, 0, numLEDS - 1);
    return true;
#else
    (void)script;
    return false;
#endif
}
void StripState::replaceAnimation(int index, ANIMATION_TYPE animType, std::map<ParameterID, float> params)
{
    if (index < 0 || index >= animations.size())
    {
        LOG_PRINTF("Invalid animation index %d\n", index);
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
bool StripState::handleTextMessage(std::string command)
{

    bool verbose = isVerbose();
    if (startsWith(command, "basic:"))
    {
        std::string script = substring(command, 6);
        return parseBasicScript(script);
    }
    if (startsWith(command, "script:"))
    {
        std::string script = substring(command, 7);
        return parseAnimationScript(script);
    }
    if (startsWith(command, "get_nodes"))
    {
        std::string nodeString = "nodes:";
        nodeString += std::to_string(stripIndex) + ":" + std::to_string(numLEDS) + ":" + std::to_string(nodes.size()) + ":";
        for (const auto &node : nodes)
        {
            nodeString += std::to_string(node.index) + "," + std::to_string(node.x) + "," + std::to_string(node.y) + "," + std::to_string(node.z) + ":";
        }
        LOG_PRINTLN(nodeString.c_str());
        return true;
    }
    if (startsWith(command, "simulate:"))
    {
        auto simParts = splitString(command, ':');
        if (simParts.size() != 2)
        {
            LOG_PRINTF("Invalid simulate command %s %d \n", command.c_str(), simParts.size());

            return false;
        }
        int simulate = toInt(simParts[1]);
        setSimulate(simulate);
        if (isVerbose())
        {
            LOG_PRINTF("Set simulate to %d\n", simulate);
        }
        return true;
    }
    if (startsWith(command, "setanimation:"))
    {
        ledState = LED_STATE_SINGLE_ANIMATION;
        auto animparts = splitString(command, ':');
        auto animName = animparts[1];
        trim(animName);

        if (animName == "POINT_CONTROL")
        {
            ledState = LED_STATE_POINT_CONTROL;
            if (isVerbose())
            {
                LOG_PRINTLN("Set LED state to POINT_CONTROL");
            }
            return true;
        }
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        if (animType == ANIMATION_TYPE_NONE)
        {
            LOG_PRINTF("Unknown animation type %s\n", animName.c_str());
            return false;
        }

        if (animparts.size() == 4)
        {
            int start = toInt(animparts[2]);
            int end = toInt(animparts[3]);
            setAnimation(animType, start, end);
            if (isVerbose())
            {
                LOG_PRINTF("Set animation %s from %d to %d \n", getAnimationName(animType).c_str(), start, end);
            }
        }
        else if (animparts.size() == 2)
        {

            setAnimation(animType, 0, getNumLEDS() - 1);
            if (isVerbose())
            {
                LOG_PRINTF("Set animation %s\n", getAnimationName(animType).c_str());
            }
        }

        return true;
    }

    if (startsWith(command, "replaceanimation:"))
    {

        auto animparts = splitString(command, ':');
        if (animparts.size() < 3)
        {
            LOG_PRINTF("Invalid replaceanimation command %s\n", command.c_str());
            return false;
        }
        auto animName = animparts[2];
        trim(animName);
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        auto whichPart = animparts[1];
        trim(whichPart);
        if (equalsIgnoreCase(whichPart, "all"))
        {
            if (isVerbose())
                LOG_PRINTF("Replacing all %d animations with %s\n", animations.size(), animName.c_str());

            for (int i = 0; i < animations.size(); i++)
            {
                replaceAnimation(i, animType);
            }
            if (isVerbose())
            {
                for (int i = 0; i < animations.size(); i++)
                {
                    auto &anim = animations[i];
                    LOG_PRINTF("Replaced animation at index %d with %s %d %d\n", i, getAnimationName(anim->getAnimationType()).c_str(), anim->getstart(), anim->getend());
                }
            }
            return true;
        }
        else
        {

            int index = toInt(whichPart);
            if (index < 0 || index >= animations.size())
            {
                LOG_PRINTF("Invalid animation index %d\n", index);
                return false;
            }
            if (animType == ANIMATION_TYPE_NONE)
            {
                LOG_PRINTF("Unknown animation type %s\n", animName.c_str());
                return false;
            }
            replaceAnimation(index, animType);
            if (isVerbose())
            {
                LOG_PRINTF("Replaced animation at index %d with %s\n", index, getAnimationName(animType).c_str());
            }
            return true;
        }
    }
    if (startsWith(command, "addanimation:"))
    {
        ledState = LED_STATE_MULTI_ANIMATION;
        auto animparts = splitString(command, ':');
        if (animparts.size() < 2)
        {
            LOG_PRINTF("Invalid addanimation command %s\n", command.c_str());
            return false;
        }
        auto animName = animparts[1];
        trim(animName);
        ANIMATION_TYPE animType = getAnimationTypeFromName(animName);
        if (animType == ANIMATION_TYPE_NONE)
        {
            LOG_PRINTF("Unknown animation type %s\n", animName.c_str());
            return false;
        }
        int start = 0;
        int end = getNumLEDS() - 1;
        if (animparts.size() == 4)
        {
            start = toInt(animparts[2]);
            end = toInt(animparts[3]);
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
        LOG_PRINTF("Invalid LED index %d\n", ledIndex);
        return;
    }
    leds[ledIndex] = CRGB(color.r, color.g, color.b);
}

void StripState::blendPixel(int index, led color)
{
    int ledIndex = (index % numLEDS + numLEDS) % numLEDS;
    if (ledIndex < 0 || ledIndex >= numLEDS)
    {
        LOG_PRINTF("Invalid LED index %d\n", ledIndex);
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

bool StripState::handleParameterMessage(parameter_message parameter)
{

    bool stripTookParam = ParameterManager::handleParameterMessage(parameter);
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
            if (animations[i]->handleParameterMessage(parameter))
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

Vec3D StripState::getLEDPosition(int ledIndex)
{
    if (nodes.empty())
    {
        return {0.0f, 0.0f, 0.0f};
    }
    if (ledIndex <= nodes.front().index)
    {
        return {nodes.front().x, nodes.front().y, nodes.front().z};
    }
    if (ledIndex >= nodes.back().index)
    {
        return {nodes.back().x, nodes.back().y, nodes.back().z};
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
            return {x, y, z};
        }
    }
    return {0.0f, 0.0f, 0.0f};
}

#endif
