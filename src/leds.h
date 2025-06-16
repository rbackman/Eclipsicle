
#pragma once
#include "shared.h"

ByteRow base64Decode(uint8_t *input, int len);
void decodeRLE(ByteRow encodedData, LedRow &dest);
String getLedStateName(LED_STATE state);
String getAnimationName(ANIMATION_TYPE type);
