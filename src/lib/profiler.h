#ifndef PROFILER_H
#define PROFILER_H

#ifdef ENABLE_PROFILER
#include <Arduino.h>
struct ProfilerStats {
    uint32_t count = 0;
    uint64_t totalLoop = 0;
    uint64_t totalLED = 0;
    unsigned long lastPrint = 0;
};

static ProfilerStats profilerStats;

inline void profilerAddLoop(uint32_t us) {
    profilerStats.totalLoop += us;
    profilerStats.count++;
}

inline void profilerAddLed(uint32_t us) {
    profilerStats.totalLED += us;
}

inline void profilerMaybePrint() {
    unsigned long now = millis();
    if (now - profilerStats.lastPrint >= 1000 && profilerStats.count > 0) {
        Serial.printf("PROFILE avgLoop=%luus avgLED=%luus\n",
                      (unsigned long)(profilerStats.totalLoop/profilerStats.count),
                      (unsigned long)(profilerStats.totalLED/profilerStats.count));
        profilerStats.totalLoop = profilerStats.totalLED = 0;
        profilerStats.count = 0;
        profilerStats.lastPrint = now;
    }
}
#else
inline void profilerAddLoop(uint32_t) {}
inline void profilerAddLed(uint32_t) {}
inline void profilerMaybePrint() {}
#endif

#endif // PROFILER_H
