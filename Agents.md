this project is for controlling an esp32 from a PC via serial communication


the cPP esp32 code is in src
the pyqt base UI is located in src/led_ui

for esp32 code:

    the main purpose of slave.cpp is to manage several LED strips. and respond to different messages from serial or ESPnow

    shared.h has most of the definitions of types
        important types are ANIMATION_TYPE,ParameterID,BoolParameter,FloatParameter,IntParameter,MenuID,LED_STATE

   

    ledManager.h deals with FastLED and initializes with a set of StripStates
    stripState.h each strip represents one connection to esp32 pin
        stripState has a RGB array connected to FastLED and manages a set of animations that can draw on the strips
    animations.h animations are derived from StripAnimation
        animation has a start and end led. setting position within animations works within that range
    parameterManager.h 
        base class that stores a set of parameters. int,float,bool
        class knows how to parse and encode data to send between devices
        most objects inherit from this to use functions getFloat,getBool.. etc
        the esp can confirm parameters to the UI to generate a parameter_map.json in src/led_ui to sync up the type definitions

    
