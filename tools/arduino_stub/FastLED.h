#ifndef FASTLED_STUB_H
#define FASTLED_STUB_H
#include <cstdint>
#include <algorithm>
struct CRGB {
    uint8_t r, g, b;
    CRGB() : r(0), g(0), b(0) {}
    CRGB(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
    bool operator==(const CRGB& other) const { return r==other.r && g==other.g && b==other.b; }
};
struct CHSV {
    uint8_t hue, sat, val;
    CHSV() : hue(0), sat(0), val(0) {}
    CHSV(uint8_t h, uint8_t s, uint8_t v) : hue(h), sat(s), val(v) {}
};
class CFastLED {
public:
    template<typename... Args> void addLeds(Args...) {}
    void show() {}
    void setBrightness(int) {}
};
static CFastLED FastLED;

inline CHSV rgb2hsv_approximate(const CRGB& rgb) {
    uint8_t r = rgb.r;
    uint8_t g = rgb.g;
    uint8_t b = rgb.b;
    uint8_t v = std::max({r, g, b});
    uint8_t minc = std::min({r, g, b});
    uint8_t delta = v - minc;
    uint8_t h = 0;
    if (delta) {
        if (v == r) {
            h = uint8_t((43 * (int(g) - int(b))) / delta);
        } else if (v == g) {
            h = uint8_t(85 + (43 * (int(b) - int(r))) / delta);
        } else {
            h = uint8_t(171 + (43 * (int(r) - int(g))) / delta);
        }
    }
    uint8_t s = v ? uint8_t(delta * 255 / v) : 0;
    return CHSV(h, s, v);
}
#endif // FASTLED_STUB_H
