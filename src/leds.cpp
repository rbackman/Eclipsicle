
#ifdef USE_LEDS

#include <Adafruit_NeoPixel.h>
#define FASTLED_ESP32_DMA
#include "FastLED.h"
#include "leds.h"
#include "sensors.h"

#define LED_TYPE WS2811
#define COLOR_ORDER GRB

// #define LED_PIN_1 2
// #define LED_PIN_2 4
// #define LED_PIN_3 2
// #define LED_PIN_4 4

led colorUtil;

ByteRow base64Decode(uint8_t *input, int len)
{
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    ByteRow ret;
    int i = 0, in_len = len, val = 0, valb = -8;
    for (int j = 0; j < in_len; j++)
    {
        char c = input[j];
        if (c == '=')
            break; // Padding character, end processing
        int pos = base64_chars.find(c);
        if (pos == std::string::npos)
            continue; // Skip invalid characters
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0)
        {
            ret.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return ret;
}

String getLedStateName(LED_STATE state)
{
    return LED_STATE_NAMES.at(state);
}

void decodeRLE(ByteRow encodedData, LedRow &dest)
{
    int destIndex = 0;

    for (int i = 0; i < encodedData.size(); i += 2)
    {
        char value = encodedData[i];
        char repeat = encodedData[i + 1];
        if (value == 0)
        {
            colorUtil = {0, 0, 0};
        }
        else
        {
            colorFromHSV(colorUtil, value / 360.0, 1, 1);
        }

        for (int j = 0; j < repeat; j++)
        {
            if (destIndex >= dest.size())
            {
                Serial.println("Error decoding RLE: destIndex out of bounds");
                return;
            }

            dest[destIndex++] = colorUtil;
        }
    }
    while (destIndex < dest.size())
    {
        dest[destIndex++] = {0, 0, 0};
    }
}

#endif