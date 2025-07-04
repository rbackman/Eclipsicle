#include "FallingBricks.h"
#include <gtest/gtest.h>

TEST(FallingBricks, ReversePixelsDrawn) {
    FallingBricksSim sim(10, 2, 1.0f, true);
    sim.update();
    bool any=false;
    for (const auto& p: sim.pixels()) {
        if (p.r || p.g || p.b) { any=true; break; }
    }
    EXPECT_TRUE(any);
    EXPECT_EQ(sim.pixels().back().r, 255);
}
