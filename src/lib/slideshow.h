#pragma once
#ifdef MESH_NET
#include "meshnet.h"
#else
class MeshnetManager;
#endif
#include <vector>
#include <Arduino.h>

struct Slide {
    String script;        // encoded animation script or name
    uint32_t duration;    // milliseconds
};

class Slideshow {
public:
    explicit Slideshow(MeshnetManager *mesh);
    void addSlide(const String &script, uint32_t duration);
    void loadScript(const String &script);
    void start();
    void update();
    bool isRunning() const { return running; }
private:
    MeshnetManager *mesh;
    std::vector<Slide> slides;
    size_t current = 0;
    uint32_t startTime = 0;
    bool running = false;
};
