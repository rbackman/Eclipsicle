#pragma once

#ifdef USE_LEDS
#include "FastLED.h"
#include "shared.h"
#include "leds.h"

#include "parameterManager.h"
#include "animations.h"
#include <memory>

#include "animations.h"
class StripState : public ParameterManager
{

private:
    int stripIndex;
    int currentAnimation = 0;
    int gravityPosition = 0;
    int numLEDS = 128;
    int simulateCount = -1;
    int counter = 0;
    float beatSize = 0;

    float scrollPos = 0;

    // nodes with LED indices and 3D positions
    std::vector<Node3D> nodes;

    std::vector<std::unique_ptr<StripAnimation>> animations;
    LED_STATE ledState = LED_STATE_IDLE;

public:
    bool isActive = true;

    CRGB *leds;
    StripState(LED_STATE state, const int numLEDS, int STRIP_INDEX,
               std::vector<Node3D> nodes = {});

    void setNumLEDS(int num)
    {
        numLEDS = num;
    }
    void addAnimation(ANIMATION_TYPE animis, int start = -1, int end = -1, std::map<ParameterID, float> params = {});
    void setSimulate(int simulateCount)
    {
        this->simulateCount = simulateCount;
    }
    void setAnimation(ANIMATION_TYPE animType, int start = -1, int end = -1, std::map<ParameterID, float> params = {})
    {
        ledState = LED_STATE_SINGLE_ANIMATION;
        animations.clear();
        addAnimation(animType, start, end, params);
    }
    int getNumLEDS()
    {
        return numLEDS;
    }
    int getNumAnimations()
    {
        return animations.size();
    }
    int getStripIndex()
    {
        return stripIndex;
    }
    void setGravityPosition(float position)
    {
        gravityPosition = (int)(position * numLEDS);
    }
    void replaceAnimation(int index, ANIMATION_TYPE animType, std::map<ParameterID, float> params = {});

    void setLEDRow(LedRow ledRow)
    {

        for (int i = 0; i < numLEDS; i++)
        {
            int r = ledRow[i].r;
            int g = ledRow[i].g;
            int b = ledRow[i].b;
            setPixel(i, r, g, b);
        }
    }
    void setLEDState(LED_STATE state)
    {
        ledState = state;
    }

    void setAll(CRGB color);
    void setAll(led color);
    bool respondToText(String command);
    bool parseAnimationScript(String script);

    void clearPixels();
    void clearPixel(int index);
    void blendPixel(int index, led color);
    void setPixel(int index, led color);
    void setPixel(int index, int r, int g, int b);

    void toggleMode();

    void update();
    String getStripState(bool verbose = false);
    String getStripStateJson(bool verbose = false);
    String getStripStateCompact();
    String getAnimationInfoJson();
    int getMidLed() const { return numLEDS / 2; }
    int getNode(int idx) const
    {
        if (idx <= 0 || idx > nodes.size())
            return 0;
        return nodes[idx - 1].index;
    }
    Vec3D getNode3D(int ledIndex) const
    {
        // find the two nodes that contain the LED index and interpolate between them
        if (ledIndex < 0 || ledIndex >= numLEDS)
        {
            printf("Invalid LED index %d\n", ledIndex);
            return {0.0f, 0.0f, 0.0f};
        }
        for (int i = 0; i < nodes.size() - 1; i++)
        {
            if (nodes[i].index <= ledIndex && nodes[i + 1].index >= ledIndex)
            {
                float t = (float)(ledIndex - nodes[i].index) / (nodes[i + 1].index - nodes[i].index);
                Vec3D pos;
                pos.x = nodes[i].x * (1.0f - t) + nodes[i + 1].x * t;
                pos.y = nodes[i].y * (1.0f - t) + nodes[i + 1].y * t;
                pos.z = nodes[i].z * (1.0f - t) + nodes[i + 1].z * t;
                return pos;
            }
        }
        // if we reach here, the LED index is outside the range of nodes

        return {0.0f, 0.0f, 0.0f};
    }
    const std::vector<Node3D> &getNodes() const
    {
        return nodes;
    }
    // get interpolated world position for a LED index
    Vec3D getLEDPosition(int ledIndex);
    int getAnimationCount()
    {
        return animations.size();
    }
    std::vector<std::unique_ptr<StripAnimation>> &getAnimations()
    {
        return animations;
    }
    bool respondToParameterMessage(parameter_message parameter);
};

#endif