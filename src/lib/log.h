#pragma once
#ifdef USE_SERIAL_PRINT
#include <Arduino.h>
#define LOG_PRINTF Serial.printf
#define LOG_PRINTLN Serial.println
#define LOG_PRINT Serial.print
#else
#include <cstdio>
#define LOG_PRINTF printf
#define LOG_PRINTLN(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOG_PRINT printf
#endif

