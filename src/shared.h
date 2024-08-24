
#pragma once

#include <vector>
#include <map>
#include <array>
#include <string>

#include <Arduino.h>
#include <cstdint>

#define PARAMETER_LIST             \
    X(PARAM_HUE)                   \
    X(PARAM_HUE_END)               \
    X(PARAM_BRIGHTNESS)            \
    X(PARAM_PARTICLE_WIDTH)        \
    X(PARAM_PARTICLE_FADE)         \
    X(PARAM_PARTICLE_UPDATE_ALL)   \
    X(PARAM_SLIDER_WIDTH)          \
    X(PARAM_SLIDER_GRAVITY)        \
    X(PARAM_VELOCITY)              \
    X(PARAM_MAX_SPEED)             \
    X(PARAM_ACCELERATION)          \
    X(PARAM_RANDOM_DRIFT)          \
    X(PARAM_TIME_SCALE)            \
    X(PARAM_SLIDER_REPEAT)         \
    X(PARAM_RAINBOW_REPEAT)        \
    X(PARAM_RAINBOW_OFFSET)        \
    X(PARAM_SLIDER_POSITION)       \
    X(PARAM_SLIDER_HUE)            \
    X(PARAM_SPAWN_RATE)            \
    X(PARAM_PARTICLE_LIFE)         \
    X(PARAM_SOUND_SCALE)           \
    X(PARAM_SCROLL_SPEED)          \
    X(PARAM_SLIDER_MULTIPLIER)     \
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
    MENU_SLIDER,
    MENU_SLIDER_COLOR_MODE,
    MENU_SLIDER_SETTINGS_MODE,
    MENU_RANDOM_MODE,
    MENU_DEBUG,
    MENU_DISPLAY_DEBUG_MODE,
    MENU_SETTINGS_DEBUG_MODE,
    MENU_MISC_DEBUG_MODE,
    MENU_LED_DEBUG_MODE,
    MENU_MASTER_LED_MODE,
    MENU_SETTINGS,
    MENU_AUDIO,
};

// map MenuType to a string  for name and MenuType for reference to the parent menu
const std::map<MenuID, std::pair<std::string, MenuID>> menuTypeMap = {
    {MENU_IDLE, {"Idle", MENU_ROOT}},
    {MENU_MAIN, {"Main", MENU_IDLE}},
    {MENU_PATTERNS, {"Patterns", MENU_MAIN}},

    {MENU_PARTICLES, {"Particles", MENU_PATTERNS}},

    {MENU_RANDOM_PARTICLES, {"RndParticles", MENU_PATTERNS}},
    {MENU_SAVE_PATTERN, {"Save", MENU_PATTERNS}},
    {MENU_RAINBOW_MODE, {"Rainbow", MENU_PATTERNS}},
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
    {MENU_LED_DEBUG_MODE, {"LEDDbg", MENU_DEBUG}},
};

struct IntParameter
{
    ParameterID id;
    std::string name;
    int value;
    int min;
    int max;
    float scale;
};

struct BoolParameter
{
    ParameterID id;
    std::string name;
    bool value;
};

// add parameters to menu
const std::vector<std::pair<MenuID, ParameterID>> parameterMenuList = {
    {MENU_PARTICLES_COLOR_MODE, PARAM_HUE},
    {MENU_PARTICLES_COLOR_MODE, PARAM_HUE_END},
    {MENU_PARTICLES_COLOR_MODE, PARAM_PARTICLE_WIDTH},
    {MENU_PARTICLES_COLOR_MODE, PARAM_TIME_SCALE},
    {MENU_PARTICLES_COLOR_MODE, PARAM_PARTICLE_FADE},
    {MENU_PARTICLES_LIFE_MODE, PARAM_PARTICLE_LIFE},
    {MENU_PARTICLES_LIFE_MODE, PARAM_PARTICLE_UPDATE_ALL},

    {MENU_PARTICLES_SPEED_MODE, PARAM_VELOCITY},
    {MENU_PARTICLES_SPEED_MODE, PARAM_ACCELERATION},
    {MENU_PARTICLES_SPEED_MODE, PARAM_MAX_SPEED},
    {MENU_PARTICLES_SPEED_MODE, PARAM_RANDOM_DRIFT},
    {MENU_PARTICLES_LIFE_MODE, PARAM_SPAWN_RATE},
    {MENU_PARTICLES_LIFE_MODE, PARAM_CYCLE},

    {MENU_RAINBOW_MODE, PARAM_SCROLL_SPEED},
    {MENU_RAINBOW_MODE, PARAM_BRIGHTNESS},
    {MENU_RAINBOW_MODE, PARAM_RAINBOW_REPEAT},
    {MENU_RAINBOW_MODE, PARAM_RAINBOW_OFFSET},
    {MENU_SLIDER_COLOR_MODE, PARAM_SLIDER_WIDTH},
    {MENU_SLIDER_COLOR_MODE, PARAM_SLIDER_REPEAT},
    {MENU_SLIDER_COLOR_MODE, PARAM_SLIDER_POSITION},
    {MENU_SLIDER_COLOR_MODE, PARAM_SLIDER_HUE},
    {MENU_SLIDER_COLOR_MODE, PARAM_SLIDER_MULTIPLIER},

    {MENU_SLIDER_SETTINGS_MODE, PARAM_SLIDER_GRAVITY},
    {MENU_SLIDER_SETTINGS_MODE, PARAM_INVERT},
    {MENU_SLIDER_SETTINGS_MODE, PARAM_CENTERED},

    {MENU_RANDOM_MODE, PARAM_RANDOM_ON},
    {MENU_RANDOM_MODE, PARAM_RANDOM_OFF},
    {MENU_RANDOM_MODE, PARAM_RANDOM_MIN},
    {MENU_RANDOM_MODE, PARAM_RANDOM_MAX},

    {MENU_MASTER_LED_MODE, PARAM_MASTER_LED_HUE},
    {MENU_MASTER_LED_MODE, PARAM_MASTER_LED_BRIGHTNESS},
    {MENU_MASTER_LED_MODE, PARAM_MASTER_LED_SATURATION},

    {MENU_DISPLAY_DEBUG_MODE, PARAM_SHOW_FPS},
    {MENU_DISPLAY_DEBUG_MODE, PARAM_DISPLAY_ACCEL},
    {MENU_DISPLAY_DEBUG_MODE, PARAM_BLACK_AND_WHITE},

    {MENU_MISC_DEBUG_MODE, PARAM_MOTOR_SPEED},
    {MENU_MISC_DEBUG_MODE, PARAM_SEQUENCE},
    {MENU_MISC_DEBUG_MODE, PARAM_SOUND_SCALE},
    {MENU_MISC_DEBUG_MODE, PARAM_RECORD_AUDIO},
    {MENU_MISC_DEBUG_MODE, PARAM_LOOP_ANIM},
    {MENU_LED_DEBUG_MODE, PARAM_CURRENT_STRIP},
    {MENU_LED_DEBUG_MODE, PARAM_CURRENT_LED},
    {MENU_LED_DEBUG_MODE, PARAM_HUE},
    {MENU_LED_DEBUG_MODE, PARAM_BRIGHTNESS},

};

static const std::vector<IntParameter> getDefaultParameters()
{

    return {
        {PARAM_HUE, "Hue", 60, 0, 360, 1.0},
        {PARAM_HUE_END, "HueEnd", 120, 0, 360, 1.0},
        {PARAM_PARTICLE_WIDTH, "Width", 10, 1, 60, 1.0},
        {PARAM_VELOCITY, "Vel", 10, 1, 100, 0.1},
        {PARAM_ACCELERATION, "Accel", 0, -10, 100, 0.1},
        {PARAM_MAX_SPEED, "MaxSpd", 1, 1, 10, 0.1},
        {PARAM_RANDOM_DRIFT, "Drift", 0, 0, 255, 1.0},

        {PARAM_SPAWN_RATE, "Spawn", 4, 1, 40, 1.0},
        {PARAM_BRIGHTNESS, "Brightness", 255, 0, 255, 1.0},
        {PARAM_PARTICLE_FADE, "Fade", 100, 0, 255, 1.0},
        {PARAM_PARTICLE_LIFE, "Life", -1, -1, 100, 1.0},
        {PARAM_TIME_SCALE, "Time", 10, 1, 100, 0.1},
        {PARAM_SLIDER_REPEAT, "Repeat", 10, 1, 100, 0.1},
        {PARAM_SLIDER_POSITION, "Pos", 0, 0, 255, 1.0},
        {PARAM_SLIDER_WIDTH, "Width", 6, 1, 60, 1.0},
        {PARAM_SLIDER_HUE, "Hue", 60, 0, 360, 1.0},
        {PARAM_RANDOM_ON, "On", 30, 0, 255, 1.0},
        {PARAM_RANDOM_OFF, "Off", 30, 0, 255, 1.0},
        {PARAM_RANDOM_MIN, "Min", 0, 0, 255, 1.0},
        {PARAM_RANDOM_MAX, "Max", 255, 0, 255, 1.0},
        {PARAM_DISPLAY_ACCEL, "Accel", 0, 0, 1, 1.0},
        {PARAM_RAINBOW_REPEAT, "Repeat", 1, 1, 10, 1.0},
        {PARAM_RAINBOW_OFFSET, "Offset", 0, 0, 255, 1.0},
        {PARAM_SOUND_SCALE, "Sound", 0, 0, 1, 1.0},
        {PARAM_SCROLL_SPEED, "Speed", 0, 0, 100, 1.0},
        {PARAM_SLIDER_MULTIPLIER, "Mult", 0, 0, 1, 1.0},
        {PARAM_CURRENT_STRIP, "CurrentStrip", 0, 0, 3, 1.0},
        {PARAM_CURRENT_LED, "CurrentLED", 1, 0, 255, 0.5},
        {PARAM_MASTER_LED_HUE, "Hue", 60, 0, 360, 1.0},
        {PARAM_MASTER_LED_BRIGHTNESS, "Brightness", 50, 0, 255, 1.0},
        {PARAM_MASTER_LED_SATURATION, "Saturation", 255, 0, 255, 1.0},
        {PARAM_MASTER_VOLUME, "Volume", 0, 0, 255, 1.0},
        {PARAM_MOTOR_SPEED, "Speed", 0, 0, 255, 1.0},
        {PARAM_BEAT, "Beat", 0, 0, 255, 1.0},
        {PARAM_BEAT_MAX_SIZE, "MaxSize", 30, 0, 255, 1.0},
        {PARAM_BEAT_FADE, "Fade", 50, 0, 255, 0.05},

    };
};

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
        {PARAM_SLIDER_GRAVITY, "Gravity", false},
        {PARAM_RECORD_AUDIO, "Record", false},
        {PARAM_PARTICLE_UPDATE_ALL, "UpdateAll", true},
    };
}

std::vector<ParameterID> getParametersForMenu(MenuID menu);

std::vector<MenuID> getChildrenOfMenu(MenuID type);
std::string getMenuPath(MenuID type, MenuID root);
const char *getMenuName(MenuID type, int MaxSize = 6);

const MenuID getParentMenu(MenuID type);

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
    int value;
    bool boolValue;
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
    int lastButtonState = HIGH;
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

#define LED_PIN_1 2
#define LED_PIN_2 4
#define LED_PIN_3 19
#define LED_PIN_4 18

enum LED_STATE
{
    LED_STATE_IDLE,
    LED_STATE_RAINBOW,
    LED_STATE_DOUBLE_RAINBOW,
    LED_STATE_SLIDER,
    LED_STATE_RANDOM,
    LED_STATE_POINT_CONTROL,
    LED_STATE_PARTICLES,
    LED_STATE_RANDOM_PARTICLES,
};
struct LEDParams
{
    int numLEDS;
    int stripIndex;
    LED_STATE startState;
    bool reverse;
};

struct LEDRig
{
    std::string name;
    MacAddress mac;
    std::vector<LEDParams> strips;
};

const std::vector<LEDRig> slaves = {
    {
        "Eclipsicle",
        {0x40, 0x91, 0x51, 0xFB, 0xB7, 0x48},
        {
            {200, 0, LED_STATE_SLIDER, false},
            {200, 1, LED_STATE_SLIDER, false},

        },
    },
    {
        "Bike",
        {0xD0, 0xEF, 0x76, 0x58, 0x45, 0xB4},
        {
            {48, 0, LED_STATE_SLIDER, false},
            {48, 1, LED_STATE_SLIDER, false},
            {18, 2, LED_STATE_SLIDER, false},
        },
    },
    {
        "SuperSpinner",
        {0x40, 0x91, 0x51, 0xFB, 0xF7, 0xBC},
        {
            {280, 0, LED_STATE_IDLE, false},
            {280, 1, LED_STATE_IDLE, false},
        },
    }};

// map to state names
const std::map<LED_STATE, String> LED_STATE_NAMES = {
    {LED_STATE_IDLE, "IDLE"},
    {LED_STATE_RAINBOW, "RAINBOW"},
    {LED_STATE_DOUBLE_RAINBOW, "DOUBLE_RAINBOW"},
    {LED_STATE_SLIDER, "SLIDER"},
    {LED_STATE_RANDOM, "RANDOM"},
    {LED_STATE_POINT_CONTROL, "POINT_CONTROL"},
    {LED_STATE_PARTICLES, "PARTICLES"},
    {LED_STATE_RANDOM_PARTICLES, "RANDOM_PARTICLES"}

};

const int LED_STATE_COUNT = LED_STATE_NAMES.size();

void colorFromHSV(led &color, float h, float s, float v);
void setVerbose(bool verb);
void printBytes(ByteRow data);

int lerp(int min, int max, int minIn, int maxIn, int value);
int interpolate(int from, int to, float t);

void sanityCheckParameters();
