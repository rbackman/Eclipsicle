#ifdef TESS_MENU
#pragma once
#include <map>
#include <string>
#include <vector>

enum MenuID
{
    MENU_ROOT = -1,
    MENU_IDLE,
    MENU_MAIN,
    MENU_PATTERNS,
    MENU_SAVE_PATTERN,
    MENU_PARTICLES,
    MENU_RANDOM_PARTICLES,
    MENU_PARTICLES_COLOR,
    MENU_PARTICLES_SPEED,
    MENU_PARTICLES_LIFE,
    MENU_RAINBOW,
    MENU_DOUBLE_RAINBOW,
    MENU_NEBULA,
    MENU_BRICKS,
    MENU_SLIDER,
    MENU_SLIDER_COLOR,
    MENU_SLIDER_SETTINGS,
    MENU_RANDOM,
    MENU_DEBUG,
    MENU_DISPLAY_DEBUG,
    MENU_SETTINGS_DEBUG,
    MENU_MISC_DEBUG,
    MENU_LED_DEBUG,
    MENU_ANIMATION_TYPE,
    MENU_SINGLE_ANIMATION,
    MENU_MULTI_ANIMATION,
    MENU_POINT_CONTROL,
    MENU_MASTER_LED,
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
    {MENU_RAINBOW, {"Rainbow", MENU_PATTERNS}},
    {MENU_NEBULA, {"Nebula", MENU_PATTERNS}},
    {MENU_BRICKS, {"Falling Bricks", MENU_PATTERNS}},
    {MENU_DOUBLE_RAINBOW, {"Double Rainbow", MENU_PATTERNS}},
    {MENU_SLIDER, {"Slider", MENU_PATTERNS}},
    {MENU_SLIDER_COLOR, {"Color", MENU_SLIDER}},
    {MENU_SLIDER_SETTINGS, {"Settings", MENU_SLIDER}},
    {MENU_RANDOM, {"Random", MENU_PATTERNS}},

    {MENU_MASTER_LED, {"LED", MENU_MAIN}},

    {MENU_PARTICLES_COLOR, {"Color", MENU_PARTICLES}},
    {MENU_PARTICLES_LIFE, {"Life", MENU_PARTICLES}},
    {MENU_PARTICLES_SPEED, {"Speed", MENU_PARTICLES}},
    {MENU_AUDIO, {"Audio", MENU_MAIN}},

    {MENU_DEBUG, {"Debug", MENU_MAIN}},
    {MENU_DISPLAY_DEBUG, {"Display", MENU_DEBUG}},
    {MENU_SETTINGS_DEBUG, {"Settings", MENU_DEBUG}},
    {MENU_MISC_DEBUG, {"Misc", MENU_DEBUG}},

};

const std::map<MenuID, std::vector<ParameterID>> menuParameters = {
    {MENU_PATTERNS, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_WIDTH, PARAM_FADE}},
    {MENU_PARTICLES, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_WIDTH, PARAM_FADE}},
    {MENU_RAINBOW, {PARAM_REPEAT, PARAM_OFFSET, PARAM_TIME_SCALE, PARAM_SCROLL_SPEED}},
    {MENU_DOUBLE_RAINBOW, {PARAM_TIME_SCALE}},
    {MENU_SLIDER_COLOR, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS}},
    {MENU_SLIDER_SETTINGS, {PARAM_GRAVITY, PARAM_REPEAT, PARAM_POSITION, PARAM_MULTIPLIER}},
    {MENU_RANDOM, {PARAM_RANDOM_MIN, PARAM_RANDOM_MAX, PARAM_RANDOM_ON, PARAM_RANDOM_OFF}},
    {MENU_NEBULA, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_NOISE_SPEED, PARAM_TIME_SCALE}},
    {MENU_BRICKS, {PARAM_HUE, PARAM_HUE_END, PARAM_BRIGHTNESS, PARAM_WIDTH}},
    {MENU_PARTICLES_COLOR, {PARAM_HUE, PARAM_HUE_END, PARAM_WIDTH, PARAM_BRIGHTNESS}},
    {MENU_PARTICLES_LIFE, {PARAM_LIFE, PARAM_FADE, PARAM_SPAWN_RATE}},
    {MENU_PARTICLES_SPEED, {PARAM_TIME_SCALE, PARAM_ACCELERATION, PARAM_VELOCITY}},
};

// MENU_TREE = {
//     "Main": {
//         "Patterns": {
//             "Particles": {
//                 "Color": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_WIDTH", "PARAM_BRIGHTNESS"],
//                 "Speed": ["PARAM_TIME_SCALE", "PARAM_ACCELERATION", "PARAM_VELOCITY"],
//                 "Life": ["PARAM_LIFE", "PARAM_FADE", "PARAM_SPAWN_RATE"]
//             },
//             "Rainbow": ["PARAM_REPEAT", "PARAM_OFFSET",
//                         "PARAM_TIME_SCALE", "PARAM_SCROLL_SPEED", "PARAM_BRIGHTNESS"],
//             "Double Rainbow": ["PARAM_TIME_SCALE"],
//             "Slider":  ["PARAM_WIDTH", "PARAM_GRAVITY",
//                         "PARAM_REPEAT", "PARAM_POSITION",
//                         "PARAM_MULTIPLIER", "PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS"],
//             "Random": ["PARAM_RANDOM_MIN", "PARAM_RANDOM_MAX",
//                        "PARAM_RANDOM_ON", "PARAM_RANDOM_OFF", "PARAM_BRIGHTNESS"

//                        ],
//             "Nebula": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS",
//                        "PARAM_NOISE_SPEED", "PARAM_TIME_SCALE", "PARAM_NOISE_SCALE"],
//             "Bricks": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS",
//                        "PARAM_WIDTH",   "PARAM_TIME_SCALE",
//                        "PARAM_HUE_VARIANCE"],
//             "Sphere": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS", "PARAM_RADIUS", "PARAM_THICKNESS", "PARAM_POS_X", "PARAM_POS_Y", "PARAM_POS_Z"],
//             "Plane":  ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS", "PARAM_POS_Y", "PARAM_THICKNESS"],
//             "Point Control": ["PARAM_CURRENT_STRIP", "PARAM_CURRENT_LED", "PARAM_HUE", "PARAM_BRIGHTNESS",]

//         },
//         "LED": {"Master LED": {}, "Animation Type": {}},
//         "Audio": {},
//         "Debug": {"Display": {}, "Settings": {}, "Misc": {}, },
//         "Animation Type": {
//             "Single Animation": {},
//             "Multi Animation": {
//                 "Current Strip": {},
//                 "All Strips": {},
//             },

//         },
//     }
// }

#endif