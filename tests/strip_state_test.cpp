#include "stripState.h"
#include "animations.h"
#include <gtest/gtest.h>

TEST(StripState, SingleColorAnimationSetsPixels) {
    setVerbose(false);
    StripState strip(LED_STATE_SINGLE_ANIMATION, 5, 0);
    strip.addAnimation(ANIMATION_TYPE_SINGLE_COLOR, 0, 4, {{PARAM_HUE, 0}, {PARAM_BRIGHTNESS, 255}});
    strip.update();
    for(int i=0;i<5;++i) {
        EXPECT_GT(strip.leds[i].r, 250);
        EXPECT_LT(strip.leds[i].g, 5);
        EXPECT_LT(strip.leds[i].b, 5);
    }
}

TEST(StripState, ReplaceAnimationKeepsCount) {
    setVerbose(false);
    StripState strip(LED_STATE_SINGLE_ANIMATION, 5, 0);
    strip.addAnimation(ANIMATION_TYPE_SINGLE_COLOR,0,4,{});
    ASSERT_EQ(strip.getAnimationCount(),1);
    strip.replaceAnimation(0, ANIMATION_TYPE_SINGLE_COLOR, {{PARAM_HUE,120}});
    ASSERT_EQ(strip.getAnimationCount(),1);
}
