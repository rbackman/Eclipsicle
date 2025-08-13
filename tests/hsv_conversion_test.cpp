#include <gtest/gtest.h>
#include "shared.h"
#include "FastLED.h"

TEST(ColorFromHSV, PrimaryColors)
{
    led c;
    colorFromHSV(c, 0.0f, 1.0f, 1.0f);
    EXPECT_GT(c.r, 250);
    EXPECT_LT(c.g, 5);
    EXPECT_LT(c.b, 5);

    colorFromHSV(c, 1.0f / 3.0f, 1.0f, 1.0f);
    EXPECT_GT(c.g, 250);
    EXPECT_LT(c.r, 5);
    EXPECT_LT(c.b, 5);

    colorFromHSV(c, 2.0f / 3.0f, 1.0f, 1.0f);
    EXPECT_GT(c.b, 240);
    EXPECT_LT(c.r, 5);
    EXPECT_LT(c.g, 5);
}

TEST(Rgb2HsvApprox, PrimaryColors)
{
    CHSV hsv = rgb2hsv_approximate(CRGB(255, 0, 0));
    EXPECT_EQ(hsv.val, 255);
    EXPECT_LT(hsv.hue, 5);

    hsv = rgb2hsv_approximate(CRGB(0, 255, 0));
    EXPECT_EQ(hsv.val, 255);
    EXPECT_GT(hsv.hue, 80);
    EXPECT_LT(hsv.hue, 90);

    hsv = rgb2hsv_approximate(CRGB(0, 0, 255));
    EXPECT_EQ(hsv.val, 255);
    EXPECT_GT(hsv.hue, 165);
    EXPECT_LT(hsv.hue, 175);
}
