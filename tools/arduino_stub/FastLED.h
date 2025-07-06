#ifndef FASTLED_STUB_H
#define FASTLED_STUB_H
#include <cstdint>
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
    return CHSV(rgb.r, 255, rgb.g);
}
#endif // FASTLED_STUB_H
