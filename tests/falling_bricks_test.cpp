#include "FallingBricks.h"
#include <gtest/gtest.h>

TEST(FallingBricks, OrientationForward) {
    FallingBricksSim sim(10, 2, 1.0f, false);
    sim.update();
    sim.update();
    EXPECT_EQ(sim.pixels().back().r, 255);
}

TEST(FallingBricks, OrientationReverse) {
    FallingBricksSim sim(10, 2, 1.0f, true);
    sim.update();
    sim.update();
    EXPECT_EQ(sim.pixels().front().r, 255);
}
