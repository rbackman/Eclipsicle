#include "FallingBricks.h"
#include <algorithm>

void FallingBricksSim::update() {
    if (brick_.pos < -width_ && stackHeight < numLEDs_) {
        brick_.width = width_;
        brick_.pos = reverse_ ? -width_ : numLEDs_ - 1 + width_;
    }
    if ((reverse_ && brick_.pos < numLEDs_) || (!reverse_ && brick_.pos >= 0)) {
        brick_.pos += (reverse_ ? 1.0f : -1.0f) * speed_;
        bool landed = reverse_
            ? brick_.pos + (width_ - 1) >= numLEDs_ - 1 - stackHeight
            : brick_.pos - (width_ - 1) <= stackHeight;
        if (landed) {
            stackHeight += width_;
            brick_.pos = -width_ - 1;
            if (stackHeight >= numLEDs_)
                stackHeight = 0;
        }
    }
    std::fill(leds.begin(), leds.end(), Led{});
    for (int i=0; i<stackHeight && i<numLEDs_; ++i) {
        leds[mapIdx(i)] = {255,255,255};
    }
    if (brick_.pos >= 0 && brick_.pos < numLEDs_) {
        for (int i=0; i<width_; ++i) {
            int idx = reverse_ ? (int)brick_.pos + i : (int)brick_.pos - i;
            if (idx >=0 && idx < numLEDs_) {
                leds[idx] = {255,255,255};
            }
        }
    }
}
