#include <gtest/gtest.h>
#include "shared.h"

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
