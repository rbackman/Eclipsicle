this project is for controlling an esp32 from a PC via serial communication


the esp32 code is in src
the UI is located in src/led_ui

for esp32 code:
    shared.h has most of the definitions of types
    the esp can sendParameters to generate a parameter_map.json in src/led_ui to sync up the type definitions
    
