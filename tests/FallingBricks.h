#ifndef FALLING_BRICKS_H
#define FALLING_BRICKS_H
#include <vector>
#include <cstdint>
struct Led {
    uint8_t r{0}, g{0}, b{0};
};
class FallingBricksSim {
    struct Brick {
        float pos{-1000.f};
        int width{1};
    } brick_;
    int numLEDs_;
    int width_;
    float speed_;
    bool reverse_;
    std::vector<Led> leds;
    int mapIdx(int idx) const { return reverse_ ? numLEDs_ - 1 - idx : idx; }
public:
    int stackHeight{0};
    FallingBricksSim(int numLEDs, int width, float speed, bool reverse)
        : numLEDs_(numLEDs), width_(width), speed_(speed), reverse_(reverse), leds(numLEDs) {}
    void update();
    const std::vector<Led>& pixels() const { return leds; }
    float brickPos() const { return brick_.pos; }
};
#endif
