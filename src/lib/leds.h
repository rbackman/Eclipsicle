
#pragma once
#include "shared.h"
#include <FastLED.h>
#include <string>

// Hardware configuration for FastLED.  These defaults are
// shared between the simulator and the LED manager so the
// colour ordering stays consistent.
#ifndef LED_TYPE
#define LED_TYPE WS2811
#endif

#ifndef COLOR_ORDER
#define COLOR_ORDER RGB
#endif

ByteRow base64Decode(uint8_t *input, int len);
void decodeRLE(ByteRow encodedData, LedRow &dest);
std::string getLedStateName(LED_STATE state);
