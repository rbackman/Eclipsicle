#include "slideshow.h"
#include <string>

Slideshow::Slideshow(MeshnetManager *mesh) : mesh(mesh) {}

void Slideshow::addSlide(const String &script, uint32_t duration) {
    slides.push_back({script, duration});
}

void Slideshow::loadScript(const String &script)
{
    slides.clear();
    int startPos = 0;
    while (startPos < script.length())
    {
        int endPos = script.indexOf('\n', startPos);
        if (endPos < 0)
            endPos = script.length();
        String line = script.substring(startPos, endPos);
        line.trim();
        if (line.length() > 0)
        {
            int colon = line.indexOf(':');
            if (colon > 0)
            {
                String name = line.substring(0, colon);
                uint32_t dur = line.substring(colon + 1).toInt();
                addSlide(name, dur);
            }
        }
        startPos = endPos + 1;
    }
}

void Slideshow::start() {
    if (slides.empty() || mesh == nullptr)
        return;
    running = true;
    current = 0;
    startTime = millis();
    mesh->sendStringToSlaves(std::string("script:") + slides[current].script.c_str());
}

void Slideshow::update() {
    if (!running || slides.empty() || mesh == nullptr)
        return;
    uint32_t now = millis();
    if (now - startTime >= slides[current].duration) {
        current = (current + 1) % slides.size();
        startTime = now;
        mesh->sendStringToSlaves(std::string("script:") + slides[current].script.c_str());
    }
}
