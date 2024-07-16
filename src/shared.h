
#pragma once

#include <vector>
#include <map>
#include <array>
#include <string>

#include <Arduino.h>
#include <cstdint>

enum LIGHT_TOWER_STATE
{

};

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
    MESSAGE_TYPE_PARAMETER
};

enum ParameterID
{
    PARAM_HUE,
    PARAM_HUE_END,
    PARAM_BRIGHTNESS,
    PARAM_PARTICLE_WIDTH,
    PARAM_PARTICLE_FADE,
    PARAM_SLIDER_WIDTH,
    PARAM_SLIDER_GRAVITY,
    PARAM_VELOCITY,
    PARAM_MAX_SPEED,
    PARAM_ACCELERATION,
    PARAM_RANDOM_DRIFT,
    PARAM_TIME_SCALE,
    PARAM_SLIDER_REPEAT,
    PARAM_SLIDER_POSITION,
    PARAM_SLIDER_HUE,
    PARAM_SHOOT,
    PARAM_SPAWN_RATE,
    PARAM_SPEED,
    PARAM_PARTICLE_LIFE,
    PARAM_PARTICLE_RESET,
    PARAM_GRAVITY,
    PARAM_SOUND_SCALE,
    PARAM_SCROLL_SPEED,
    PARAM_MULTIPLIER,
    PARAM_RANDOM_ON,
    PARAM_RANDOM_OFF,
    PARAM_RANDOM_MIN,
    PARAM_RANDOM_MAX,
    PARAM_INVERT,
    PARAM_CENTERED,
    PARAM_BLACK_AND_WHITE,
    PARAM_LOOP_ANIM,
    PARAM_CYCLE,
    PARAM_ACCEL_MODE,
    PARAM_SEQUENCE,
    PARAM_TARGET,
    PARAM_SHOW_FPS,
    PARAM_SPREAD,
    PARAM_COLOR_VARIATION,
    PARAM_DEATH_RATE,
    PARAM_DISPLAY_ACCEL,
    PARAM_RECORD_AUDIO,
};

// map ParameterType to string for full name and short display name under 6 chars

const char *getParameterName(ParameterID type);

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
    MENU_SLIDER_MODE,
    MENU_RANDOM_MODE,
    MENU_DEBUG,
    MENU_SETTINGS,
    MENU_AUDIO,
};

// map MenuType to a string  for name and MenuType for reference to the parent
const std::map<MenuID, std::pair<std::string, MenuID>> menuTypeMap = {
    {MENU_IDLE, {"Idle", MENU_ROOT}},
    {MENU_MAIN, {"Main", MENU_IDLE}},
    {MENU_PATTERNS, {"Patterns", MENU_MAIN}},

    {MENU_PARTICLES, {"Particles", MENU_PATTERNS}},

    {MENU_RANDOM_PARTICLES, {"RndParticles", MENU_PATTERNS}},
    {MENU_SAVE_PATTERN, {"Save", MENU_PATTERNS}},
    {MENU_RAINBOW_MODE, {"Rainbow", MENU_PATTERNS}},
    {MENU_DOUBLE_RAINBOW_MODE, {"Double Rainbow", MENU_PATTERNS}},
    {MENU_SLIDER_MODE, {"Slider", MENU_PATTERNS}},
    {MENU_RANDOM_MODE, {"Random", MENU_PATTERNS}},
    
    {MENU_PARTICLES_COLOR_MODE, {"Color", MENU_PARTICLES}},
    {MENU_PARTICLES_LIFE_MODE, {"Life", MENU_PARTICLES}},
    {MENU_PARTICLES_SPEED_MODE, {"Speed", MENU_PARTICLES}},
    {MENU_AUDIO, {"Audio", MENU_MAIN}},

    {MENU_DEBUG, {"Debug", MENU_MAIN}}};
;

struct Parameter
{
    MenuID menu;
    std::string name;
    int value;
    int min;
    int max;
};

const std::map<ParameterID, Parameter> parameterMenuMap = {
    {PARAM_HUE, {MENU_PARTICLES_COLOR_MODE, "Hue", 0, 0, 360}},
    {PARAM_HUE_END, {MENU_PARTICLES_COLOR_MODE, "HueEnd", 0, 0, 360}},
    {PARAM_TIME_SCALE, {MENU_PARTICLES_COLOR_MODE, "Time", 1, -10, 10}},
    {PARAM_PARTICLE_WIDTH, {MENU_PARTICLES_COLOR_MODE, "Width", 1, 1, 60}},
    {PARAM_SHOOT, {MENU_PARTICLES_COLOR_MODE, "Shoot", 0, 0, 255}},

    {PARAM_VELOCITY, {MENU_PARTICLES_SPEED_MODE, "Vel", 1, 1, 100}},
    {PARAM_ACCELERATION, {MENU_PARTICLES_SPEED_MODE, "Accel", -1, 1, 10}},
    {PARAM_MAX_SPEED, {MENU_PARTICLES_SPEED_MODE, "MaxSpd", 100, 1, 100}},
    {PARAM_RANDOM_DRIFT, {MENU_PARTICLES_SPEED_MODE, "Drift", 0, 0, 255}},

    

    {PARAM_SPAWN_RATE, {MENU_PARTICLES_LIFE_MODE, "Spawn", 20, 1, 40}},
    {PARAM_BRIGHTNESS, {MENU_PARTICLES_LIFE_MODE, "Brightness", 255, 0, 255}},
    {PARAM_PARTICLE_FADE, {MENU_PARTICLES_LIFE_MODE, "Fade", 0, 0, 255}},
    {PARAM_PARTICLE_LIFE, {MENU_PARTICLES_LIFE_MODE, "Life", -1, -1, 100}},
    {PARAM_PARTICLE_RESET, {MENU_PARTICLES_LIFE_MODE, "Reset", 0, 0, 1}},

    {PARAM_SLIDER_REPEAT, {MENU_SLIDER_MODE, "Repeat", 1, 1, 10}},
    {PARAM_SLIDER_POSITION, {MENU_SLIDER_MODE, "Pos", 0, 0, 255}},
    {PARAM_SLIDER_HUE, {MENU_SLIDER_MODE, "Hue", 0, 0, 360}},
    {PARAM_SLIDER_WIDTH, {MENU_SLIDER_MODE, "Width", 1, 1, 150}},
    {PARAM_SLIDER_GRAVITY, {MENU_SLIDER_MODE, "Gravity", 0, 0, 1}},
    {PARAM_RANDOM_ON, {MENU_RANDOM_MODE, "On", 30, 0, 255}},
    {PARAM_RANDOM_OFF, {MENU_RANDOM_MODE, "Off", 30, 0, 255}},
    {PARAM_RANDOM_MIN, {MENU_RANDOM_MODE, "Min", 0, 0, 255}},
    {PARAM_RANDOM_MAX, {MENU_RANDOM_MODE, "Max", 255, 0, 255}},
    {PARAM_DISPLAY_ACCEL, {MENU_DEBUG, "Accel", 0, 0, 1}},

    {PARAM_RECORD_AUDIO, {MENU_AUDIO, "Record", 0, 0, 1}},

};

std::vector<MenuID> getChildrenOfMenu(MenuID type);
std::string getMenuPath(MenuID type, MenuID root);
const char *getMenuName(MenuID type);

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

using MacAddress = std::array<uint8_t, 6>; // Change this line
using MacAddresses = std::map<std::string, MacAddress>;

void colorFromHSV(led &color, float h, float s, float v);
void setVerbose(bool verb);
void printBytes(ByteRow data);

int lerp(int min, int max, int minIn, int maxIn, int value);
int interpolate(int from, int to, float t);