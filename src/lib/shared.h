
// shared.h
#pragma once

#include <vector>
#include <map>
#include <array>
#include <string>
#include <cstdint>

#define PARAMETER_LIST             \
    X(PARAM_HUE)                   \
    X(PARAM_HUE_END)               \
    X(PARAM_BRIGHTNESS)            \
    X(PARAM_WIDTH)                 \
    X(PARAM_FADE)                  \
    X(PARAM_PARTICLE_UPDATE_ALL)   \
    X(PARAM_GRAVITY)               \
    X(PARAM_VELOCITY)              \
    X(PARAM_MAX_SPEED)             \
    X(PARAM_ACCELERATION)          \
    X(PARAM_RANDOM_DRIFT)          \
    X(PARAM_TIME_SCALE)            \
    X(PARAM_REPEAT)                \
    X(PARAM_OFFSET)                \
    X(PARAM_POSITION)              \
    X(PARAM_SPAWN_RATE)            \
    X(PARAM_LIFE)                  \
    X(PARAM_SOUND_SCALE)           \
    X(PARAM_SCROLL_SPEED)          \
    X(PARAM_MULTIPLIER)            \
    X(PARAM_RANDOM_ON)             \
    X(PARAM_RANDOM_OFF)            \
    X(PARAM_RANDOM_MIN)            \
    X(PARAM_RANDOM_MAX)            \
    X(PARAM_INVERT)                \
    X(PARAM_CENTERED)              \
    X(PARAM_BLACK_AND_WHITE)       \
    X(PARAM_LOOP_ANIM)             \
    X(PARAM_CYCLE)                 \
    X(PARAM_SEQUENCE)              \
    X(PARAM_SHOW_FPS)              \
    X(PARAM_DISPLAY_ACCEL)         \
    X(PARAM_RECORD_AUDIO)          \
    X(PARAM_CURRENT_STRIP)         \
    X(PARAM_CURRENT_LED)           \
    X(PARAM_MASTER_LED_HUE)        \
    X(PARAM_MASTER_LED_BRIGHTNESS) \
    X(PARAM_MASTER_LED_SATURATION) \
    X(PARAM_MASTER_VOLUME)         \
    X(PARAM_MOTOR_SPEED)           \
    X(PARAM_BEAT)                  \
    X(PARAM_BEAT_MAX_SIZE)         \
    X(PARAM_BEAT_FADE)             \
    X(PARAM_NOISE_SCALE)           \
    X(PARAM_NOISE_SPEED)           \
    X(PARAM_ANIMATION_TYPE)        \
    X(PARAM_REVERSE)               \
    X(PARAM_HUE_VARIANCE)          \
    X(PARAM_POS_X)                 \
    X(PARAM_POS_Y)                 \
    X(PARAM_POS_Z)                 \
    X(PARAM_RADIUS)                \
    X(PARAM_THICKNESS)             \
    X(PARAM_UNKNOWN)

// This is the maximum value that your ADC can read. For the ESP32, this is typically 4095
#define ADC_MAX 2600
#define ADC_MIN 360

// This is the ADC attenuation. The ESP32 ADC can be a bit weird, so you might need to experiment with this
#define ADC_ATTEN ADC_ATTEN_DB_11

typedef struct
{
    int randomOn = 30;
    int randomOff = 30;
    int randomMin = 0;
    int randomMax = 255;
} RandomParams;

enum SensorType
{
    DIAL,
    SLIDER,
    IR_RANGEFINDER,
    BUTTON,
    SONAR,
    ACCELEROMETER,
    GYROSCOPE,
};

enum SensorID
{
    SLIDER1,
    SLIDER2,
    SLIDER3,
    SLIDER4,
    SLIDER5,
    DIAL1,
    DIAL2,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_TRIGGER,
    ACCEL_X,
    ACCEL_Y,
    ACCEL_Z,
    GYRO_X,
    GYRO_Y,
    GYRO_Z,
    AUDIO_AMP,
    AUDIO_FREQ,
    AUDIO_BEAT,
    RANGEFINDER,
};

const SensorID sliderIDs[] = {SLIDER1, SLIDER2, SLIDER3, SLIDER4, SLIDER5};

bool isSlider(SensorID id);
const std::map<SensorID, std::string> sensorIDMap = {
    {SLIDER1, "Slider1"},
    {SLIDER2, "Slider2"},
    {SLIDER3, "Slider3"},
    {SLIDER4, "Slider4"},
    {SLIDER5, "Slider5"},
    {DIAL1, "Dial1"},
    {DIAL2, "Dial2"},
    {BUTTON_UP, "Up"},
    {BUTTON_DOWN, "Down"},
    {BUTTON_TRIGGER, "Trigger"},
    {BUTTON_RIGHT, "Right"},
    {BUTTON_LEFT, "Left"},

    {ACCEL_X, "AccelX"},
    {ACCEL_Y, "AccelY"},
    {ACCEL_Z, "AccelZ"},
    {GYRO_X, "GyroX"},
    {GYRO_Y, "GyroY"},
    {GYRO_Z, "GyroZ"},
    {AUDIO_AMP, "AudioAmp"},
    {AUDIO_FREQ, "AudioFreq"},
    {AUDIO_BEAT, "AudioBeat"}};

const char *getSensorName(SensorID id);

enum MESSAGE_TYPE
{
    MESSAGE_TYPE_TEXT,
    MESSAGE_TYPE_IMAGE,
    MESSAGE_TYPE_SENSOR,
    MESSAGE_TYPE_PARAMETER,
    MESSAGE_TYPE_BOOL_PARAMETER,
};

enum ParameterType
{
    PARAMETER_TYPE_INT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_BOOL,
    PARAMETER_TYPE_STRING,
};
enum ParameterID
{
#define X(name) name,
    PARAMETER_LIST
#undef X
};
std::vector<std::string> getParameterNames();
std::string getParameterName(ParameterID id);
enum MenuID
{
    MENU_ROOT = -1,
    MENU_IDLE,
    MENU_MAIN,
    MENU_PATTERNS,
    MENU_SAVE_PATTERN,
    MENU_PARTICLES,
    MENU_RANDOM_PARTICLES,
    MENU_PARTICLES_COLOR_MODE,
    MENU_PARTICLES_SPEED_MODE,
    MENU_PARTICLES_LIFE_MODE,
    MENU_RAINBOW_MODE,
    MENU_DOUBLE_RAINBOW_MODE,
    MENU_NEBULA_MODE,
    MENU_BRICKS_MODE,
    MENU_SLIDER,
    MENU_SLIDER_COLOR_MODE,
    MENU_SLIDER_SETTINGS_MODE,
    MENU_RANDOM_MODE,
    MENU_DEBUG,
    MENU_DISPLAY_DEBUG_MODE,
    MENU_SETTINGS_DEBUG_MODE,
    MENU_MISC_DEBUG_MODE,
    MENU_LED_DEBUG_MODE,
    MENU_ANIMATION_TYPE,
    MENU_SINGLE_ANIMATION,
    MENU_MULTI_ANIMATION,
    MENU_POINT_CONTROL,
    MENU_MASTER_LED_MODE,
    MENU_SETTINGS,
    MENU_AUDIO,
};

// map MenuType to a string  for name and MenuType for reference to the parent menu
const std::map<MenuID, std::pair<std::string, MenuID>> menuTypeMap = {
    {MENU_IDLE, {"Idle", MENU_ROOT}},
    {MENU_MAIN, {"Main", MENU_IDLE}},
    {MENU_PATTERNS, {"Patterns", MENU_MAIN}},
    {MENU_ANIMATION_TYPE, {"Animation Type", MENU_MAIN}},
    {MENU_SINGLE_ANIMATION, {"Single Animation", MENU_ANIMATION_TYPE}},
    {MENU_MULTI_ANIMATION, {"Multi Animation", MENU_ANIMATION_TYPE}},
    {MENU_POINT_CONTROL, {"Point Control", MENU_ANIMATION_TYPE}},
    {MENU_PARTICLES, {"Particles", MENU_PATTERNS}},

    {MENU_RANDOM_PARTICLES, {"RndParticles", MENU_PATTERNS}},
    {MENU_SAVE_PATTERN, {"Save", MENU_PATTERNS}},
    {MENU_RAINBOW_MODE, {"Rainbow", MENU_PATTERNS}},
    {MENU_NEBULA_MODE, {"Nebula", MENU_PATTERNS}},
    {MENU_BRICKS_MODE, {"Falling Bricks", MENU_PATTERNS}},
    {MENU_DOUBLE_RAINBOW_MODE, {"Double Rainbow", MENU_PATTERNS}},
    {MENU_SLIDER, {"Slider", MENU_PATTERNS}},
    {MENU_SLIDER_COLOR_MODE, {"Color", MENU_SLIDER}},
    {MENU_SLIDER_SETTINGS_MODE, {"Settings", MENU_SLIDER}},
    {MENU_RANDOM_MODE, {"Random", MENU_PATTERNS}},

    {MENU_MASTER_LED_MODE, {"LED", MENU_MAIN}},

    {MENU_PARTICLES_COLOR_MODE, {"Color", MENU_PARTICLES}},
    {MENU_PARTICLES_LIFE_MODE, {"Life", MENU_PARTICLES}},
    {MENU_PARTICLES_SPEED_MODE, {"Speed", MENU_PARTICLES}},
    {MENU_AUDIO, {"Audio", MENU_MAIN}},

    {MENU_DEBUG, {"Debug", MENU_MAIN}},
    {MENU_DISPLAY_DEBUG_MODE, {"Display", MENU_DEBUG}},
    {MENU_SETTINGS_DEBUG_MODE, {"Settings", MENU_DEBUG}},
    {MENU_MISC_DEBUG_MODE, {"Misc", MENU_DEBUG}},

};

struct IntParameter
{
    ParameterID id;
    std::string name;
    int value;
    int min;
    int max;
};

struct BoolParameter
{
    ParameterID id;
    std::string name;
    bool value;
};

struct FloatParameter
{
    ParameterID id;
    std::string name;
    float value;
    float min;
    float max;
};

// // add parameters to menu

static const std::vector<IntParameter> getDefaultIntParameters()
{

    return {
        {PARAM_HUE, "Hue", 60, 0, 360},
        {PARAM_HUE_END, "HueEnd", 120, 0, 360},
        {PARAM_HUE_VARIANCE, "HueVar", 0, 0, 180},
        {PARAM_WIDTH, "Width", 5, 1, 60},

        {PARAM_RANDOM_DRIFT, "Drift", 0, 0, 255},
        {PARAM_ANIMATION_TYPE, "AnimType", 0, 0, 255},
        {PARAM_SPAWN_RATE, "Spawn", 4, 1, 40},
        {PARAM_BRIGHTNESS, "Brightness", 255, 0, 255},
        {PARAM_FADE, "Fade", 100, 0, 255},
        {PARAM_LIFE, "Life", -1, -1, 100},

        {PARAM_POSITION, "Pos", 0, -255, 255},

        {PARAM_RANDOM_ON, "On", 30, 0, 255},
        {PARAM_RANDOM_OFF, "Off", 30, 0, 255},
        {PARAM_RANDOM_MIN, "Min", 0, 0, 255},
        {PARAM_RANDOM_MAX, "Max", 255, 0, 255},
        {PARAM_DISPLAY_ACCEL, "Accel", 0, 0, 1},

        {PARAM_OFFSET, "Offset", 0, 0, 255},
        {PARAM_SOUND_SCALE, "Sound", 0, 0, 1},

        {PARAM_MULTIPLIER, "Mult", 0, 0, 1},
        {PARAM_CURRENT_STRIP, "CurrentStrip", -1, -1, 4},
        {PARAM_CURRENT_LED, "CurrentLED", 1, 0, 255},
        {PARAM_MASTER_LED_HUE, "Hue", 60, 0, 360},
        {PARAM_MASTER_LED_BRIGHTNESS, "Brightness", 50, 0, 255},
        {PARAM_MASTER_LED_SATURATION, "Saturation", 255, 0, 255},
        {PARAM_MASTER_VOLUME, "Volume", 0, 0, 255},
        {PARAM_MOTOR_SPEED, "Speed", 0, 0, 255},
        {PARAM_BEAT, "Beat", 0, 0, 255},
        {PARAM_BEAT_MAX_SIZE, "MaxSize", 30, 0, 255},
        {PARAM_BEAT_FADE, "Fade", 50, 0, 255},
    };
}

static const std::vector<BoolParameter> getDefaultBoolParameters()
{

    return {
        {PARAM_INVERT, "Invert", false},
        {PARAM_CENTERED, "Centered", false},
        {PARAM_BLACK_AND_WHITE, "B&W", false},
        {PARAM_LOOP_ANIM, "Loop", false},
        {PARAM_CYCLE, "Cycle", false},
        {PARAM_SEQUENCE, "Seq", false},
        {PARAM_SHOW_FPS, "FPS", false},
        {PARAM_GRAVITY, "Gravity", false},
        {PARAM_RECORD_AUDIO, "Record", false},
        {PARAM_PARTICLE_UPDATE_ALL, "UpdateAll", true},
        {PARAM_REVERSE, "Dir", false},

    };
}

static const std::vector<FloatParameter> getDefaultFloatParameters()
{

    return {
        {PARAM_GRAVITY, "Gravity", 0.0, 0.0, 1.0},
        {PARAM_MULTIPLIER, "Mult", 1.0, 0.0, 1.0},
        {PARAM_SCROLL_SPEED, "Speed", 1.0, 0.0, 100.0},
        {PARAM_SOUND_SCALE, "Sound", 1.0, 0.0, 1.0},
        {PARAM_TIME_SCALE, "Time", 1.0, 0, 200.0},
        {PARAM_REPEAT, "Repeat", 0.9, 0.1, 10.0},
        {PARAM_VELOCITY, "Vel", 20.0, -100, 100.0},
        {PARAM_ACCELERATION, "Accel", 0.0, -10.0, 100.0},
        {PARAM_MAX_SPEED, "MaxSpd", 1.0, 1.0, 10.0},
        {PARAM_REPEAT, "Repeat", 1.0, 1.0, 10.0},

        {PARAM_NOISE_SCALE, "NScl", 0.2, 0.01, 2.0},
        {PARAM_NOISE_SPEED, "NSpd", 0.1, 0.0, 5.0},
        {PARAM_POS_X, "PosX", 0.0, -100.0, 100.0},
        {PARAM_POS_Y, "PosY", 0.0, -100.0, 100.0},
        {PARAM_POS_Z, "PosZ", 0.0, -100.0, 100.0},
        {PARAM_RADIUS, "Radius", 10.0, 0.0, 200.0},
        {PARAM_THICKNESS, "Thick", 1.0, 0.0, 50.0},
    };
}

bool isBoolParameter(ParameterID id);

bool isFloatParameter(ParameterID id);
bool isIntParameter(ParameterID id);

// std::vector<ParameterID> getParametersForMenu(MenuID menu);

std::vector<MenuID> getChildrenOfMenu(MenuID type);
std::string getMenuPath(MenuID type, MenuID root);
const char *getMenuName(MenuID type, int MaxSize = 6);

const MenuID getParentMenu(MenuID type);

std::vector<std::string> splitString(const std::string &path, char delimiter);

// Maximum sizes
#define MAX_TEXT_SIZE 144
#define MAX_PIXLE_BYTES_SIZE 144

// Struct for text messages
typedef struct text_message
{
    MESSAGE_TYPE type;
    int numBytes;
    char text[MAX_TEXT_SIZE];
} text_message;

typedef struct sensor_message
{
    MESSAGE_TYPE type;
    SensorType sensorType;
    SensorID sensorId;
    int value;

} sensor_message;

typedef struct parameter_message
{
    MESSAGE_TYPE type;
    int value = 0;
    bool boolValue = false;
    float floatValue = 0.0f;
    ParameterID paramID;
} parameter_message;

// Struct for image data messages
typedef struct image_message
{
    MESSAGE_TYPE type;
    int width;
    int row;
    int size;
    int numBytes;
    uint8_t pixleBytes[MAX_PIXLE_BYTES_SIZE];
} image_message;

struct SensorState
{
    SensorType type;
    SensorID sensorID;
    int pin;

    bool invert = false;
    int value = 0;

    bool initialized = false;
    bool changed = false;
    int lastValues[5] = {0, 0, 0, 0, 0};
    int lastButtonState = 1;
    unsigned long lastDebounceTime = 0;
    int tolerance = 1;
    SensorState(SensorType t, int p, SensorID n) : type(t), pin(p), sensorID(n) {}
    SensorState(SensorType t, int p, SensorID n, int tol) : type(t), pin(p), sensorID(n), tolerance(tol) {}
};
struct led
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef std::vector<led> LedRow;
typedef std::vector<uint8_t> ByteRow;
typedef std::vector<LedRow> LedMatrix;
typedef std::vector<SensorState> SensorGrid;

bool isVerbose();

using MacAddress = std::array<uint8_t, 6>;
using MacAddresses = std::map<std::string, MacAddress>;

#define HALL_EFFECT_PIN 34

#define MAX_LEDS_PER_STRIP 200

#define LED_PIN_1 23
#define LED_PIN_2 5
#define LED_PIN_3 19
#define LED_PIN_4 18

enum LED_STATE
{
    LED_STATE_IDLE,
    LED_STATE_SINGLE_ANIMATION,
    LED_STATE_MULTI_ANIMATION,
    LED_STATE_POINT_CONTROL,
};

#define ANIMATION_LIST                 \
    X(ANIMATION_TYPE_NONE)             \
    X(ANIMATION_TYPE_PARTICLES)        \
    X(ANIMATION_TYPE_RAINBOW)          \
    X(ANIMATION_TYPE_DOUBLE_RAINBOW)   \
    X(ANIMATION_TYPE_SLIDER)           \
    X(ANIMATION_TYPE_RANDOM)           \
    X(ANIMATION_TYPE_BRICKS)           \
    X(ANIMATION_TYPE_NEBULA)           \
    X(ANIMATION_TYPE_RANDOM_PARTICLES) \
    X(ANIMATION_TYPE_SINGLE_COLOR)     \
    X(ANIMATION_TYPE_SPHERE)           \
    X(ANIMATION_TYPE_PLANE)            \
    X(ANIMATION_TYPE_IDLE)

enum ANIMATION_TYPE
{
#define X(name) name,
    ANIMATION_LIST
#undef X
};

std::string getAnimationName(ANIMATION_TYPE type);
std::vector<std::string> getAnimationNames();
struct AnimationParams
{
    ANIMATION_TYPE type;
    int start = 0;
    int end = -1; // -1 means all LEDs

    std::map<ParameterID, float> params = {};
};

struct Node3D
{
    int index = 0; // LED index corresponding to this node
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};
struct Vec3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};
struct LEDParams
{

    int stripIndex;
    int numLEDS;
    LED_STATE state;
    std::vector<AnimationParams> animations;
    // nodes along the strip with optional 3D coordinates
    std::vector<Node3D> nodes = {};
};

struct LEDRig
{
    // presets for devices with different LED configurations
    std::string name;
    MacAddress mac;
    std::vector<LEDParams> strips;
};

// map to state names
const std::map<LED_STATE, std::string> LED_STATE_NAMES = {
    {LED_STATE_IDLE, "IDLE"},
    {LED_STATE_POINT_CONTROL, "POINTCONTROL"},
    {LED_STATE_SINGLE_ANIMATION, "SINGLEANIMATION"},
    {LED_STATE_MULTI_ANIMATION, "MULTIANIMATION"}};

const std::map<ANIMATION_TYPE, std::string> ANIMATION_TYPE_NAMES = {
    {ANIMATION_TYPE_NONE, "NONE"},
    {ANIMATION_TYPE_PARTICLES, "PARTICLES"},
    {ANIMATION_TYPE_RAINBOW, "RAINBOW"},

    {ANIMATION_TYPE_DOUBLE_RAINBOW, "DOUBLERAINBOW"},
    {ANIMATION_TYPE_SLIDER, "SLIDER"},
    {ANIMATION_TYPE_RANDOM, "RANDOM"},
    {ANIMATION_TYPE_BRICKS, "BRICKS"},
    {ANIMATION_TYPE_NEBULA, "NEBULA"},
    {ANIMATION_TYPE_RANDOM_PARTICLES, "RANDOMPARTICLES"},
    {ANIMATION_TYPE_SINGLE_COLOR, "SINGLECOLOR"},
    {ANIMATION_TYPE_SPHERE, "SPHERE"},
    {ANIMATION_TYPE_PLANE, "PLANE"}};

const int LED_STATE_COUNT = LED_STATE_NAMES.size();

// const LEDRig eclipse = {
//     "Eclipsicle",
//     {0x40, 0x91, 0x51, 0xFB, 0xB7, 0x48},
//     {
//         {0, 164, LED_STATE_SINGLE_ANIMATION, {
//                                                  {ANIMATION_TYPE_RAINBOW, 0, 163},
//                                              },
//          {{32, 0, 0, 0}, {65, 0, 0, 0}, {90, 0, 0, 0}}},
//         {1, 200, LED_STATE_SINGLE_ANIMATION, {
//                                                  {ANIMATION_TYPE_SLIDER, 0, 200},
//                                              },
//          {{32, 0, 0, 0}, {65, 0, 0, 0}, {90, 0, 0, 0}}},
//     },

// };
// const LEDRig tesseratic = {
//     "Tesseratica",
//     {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
//     {
//         {
//             0,
//             122,
//             LED_STATE_MULTI_ANIMATION,
//             {{
//                  ANIMATION_TYPE_PARTICLES,
//                  -1,
//                  -1,
//                  {{PARAM_HUE, 100}, {PARAM_HUE_END, 200}, {PARAM_TIME_SCALE, 50}},
//              },
//              {{0, 49.4, -54.8, 54.8}, {49, 21, -26.5, 26.5}, {73, -21, -26.5, 26.5}, {94, -49.4, -54.8, 54.8}, {122, 49.4, -54.8, 54.8}}},
//         },
//         {1, 122, LED_STATE_MULTI_ANIMATION, {
//                                                 {ANIMATION_TYPE_BRICKS, -1, -1, {{PARAM_HUE, 100}}},
//                                             },
//          {{0, 54.785, -54.8, 49.3}, {49, 54.8, -54.8, -49.4}, {73, 26.5, -26.5, -21}, {94, 26.5, -26.5, 21}, {122, 54.785, -54.8, 49.3}}},

//         {2, 122, LED_STATE_MULTI_ANIMATION, {
//                                                 {ANIMATION_TYPE_BRICKS, -1, -1, {{PARAM_HUE, 100}}},
//                                             },
//          {{0, 49.4, -54.8, -54.8}, {49, -49.4, -54.8, -54.8}, {73, -21, -26.5, -26.5}, {94, 21, -26.5, -26.5}, {122, 49.4, -54.8, -54.8}}},

//     },

// };

// const LEDRig squareLoop = {
//     "SquareLoop",
//     {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
//     {
//         {0, 40, LED_STATE_SINGLE_ANIMATION, {
//                                                 {ANIMATION_TYPE_RAINBOW, 0, 39},
//                                             },
//          {
//              {0, 0.0f, 0.0f, 0.0f},
//              {10, 1.0f, 0.0f, 0.0f},
//              {20, 1.0f, 1.0f, 0.0f},
//              {30, 0.0f, 1.0f, 0.0f},
//          }},
//     },

// };

//   int stripIndex;
// int numLEDS;
// LED_STATE state;
// std::vector<AnimationParams> animations;
// // nodes along the strip with optional 3D coordinates
// std::vector<Node3D> nodes = {};
void makeRig(const std::string &name, const MacAddress &mac);
void addStripToRig(const std::string &name, int stripIndex, int numLEDS, LED_STATE state, std::vector<AnimationParams> animations = {}, std::vector<Node3D> nodes = {});
std::vector<LEDRig> getLEDRigs();
LEDRig *getLEDRig(const std::string &name);
void colorFromHSV(led &color, float h, float s, float v);
void setVerbose(bool verb);
void printBytes(ByteRow data);

int lerp(int min, int max, int minIn, int maxIn, int value);
int interpolate(int from, int to, float t);

void sanityCheckParameters();
