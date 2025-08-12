cmake_minimum_required(VERSION 3.10)
project(led_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(led_sim SHARED
    strip_sim.cpp
    ../../src/lib/animations.cpp
    ../../src/lib/stripState.cpp
    ../../src/lib/parameterManager.cpp
    ../../src/lib/shared.cpp
)

target_include_directories(led_sim PRIVATE
    ../../src/lib
    ../arduino_stub
)

target_compile_definitions(led_sim PRIVATE USE_LEDS)

