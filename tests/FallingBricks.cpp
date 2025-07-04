#include "FallingBricks.h"
#include <algorithm>

void FallingBricksSim::update() {
    if (brickPos < -width_ && stackHeight < numLEDs_) {
        brickPos = reverse_ ? -width_ : numLEDs_ - 1 + width_;
    }
    if ((reverse_ && brickPos < numLEDs_) || (!reverse_ && brickPos >= 0)) {
        brickPos += (reverse_ ? 1.0f : -1.0f) * speed_;
        bool landed = reverse_
            ? brickPos + (width_ - 1) >= numLEDs_ - 1 - stackHeight
            : brickPos - (width_ - 1) <= stackHeight;
        if (landed) {
            stackHeight += width_;
            brickPos = -width_ - 1;
            if (stackHeight >= numLEDs_)
                stackHeight = 0;
        }
    }
    std::fill(leds.begin(), leds.end(), Led{});
    for (int i=0; i<stackHeight && i<numLEDs_; ++i) {
        leds[mapIdx(i)] = {255,255,255};
    }
    if (brickPos >= 0 && brickPos < numLEDs_) {
        for (int i=0; i<width_; ++i) {
            int idx = reverse_ ? (int)brickPos + i : (int)brickPos - i;
            if (idx >=0 && idx < numLEDs_) {
                leds[mapIdx(idx)] = {255,255,255};
            }
        }
    }
}
