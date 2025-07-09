
#pragma once
#include "shared.h"
#include <Arduino.h>
void printBytes(ByteRow data)
{
    Serial.println("bytes -->");
    for (int i = 0; i < data.size(); ++i)
    {
        Serial.printf("%d ", data[i]);
    }
    Serial.println("<-- as bytes");
}