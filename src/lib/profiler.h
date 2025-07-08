#ifndef PROFILER_H
#define PROFILER_H

#ifdef ENABLE_PROFILER
#include <Arduino.h>
struct ProfilerStats {
    uint32_t count = 0;
    uint64_t totalLoop = 0;
    uint64_t totalLED = 0;
    uint32_t maxLoop = 0;
    uint32_t maxLED = 0;
    unsigned long lastPrint = 0;
};

static ProfilerStats profilerStats;

inline void profilerAddLoop(uint32_t us) {
    profilerStats.totalLoop += us;
    if (us > profilerStats.maxLoop) {
        profilerStats.maxLoop = us;
    }
    profilerStats.count++;
}

inline void profilerAddLed(uint32_t us) {
    profilerStats.totalLED += us;
    if (us > profilerStats.maxLED) {
        profilerStats.maxLED = us;
    }
}

inline void profilerMaybePrint() {
    unsigned long now = millis();
    if (now - profilerStats.lastPrint >= 1000 && profilerStats.count > 0) {
        Serial.printf("PROFILE fps=%lu avgLoop=%luus maxLoop=%luus avgLED=%luus maxLED=%luus\n",
                      profilerStats.count,
                      (unsigned long)(profilerStats.totalLoop / profilerStats.count),
                      (unsigned long)profilerStats.maxLoop,
                      (unsigned long)(profilerStats.totalLED / profilerStats.count),
                      (unsigned long)profilerStats.maxLED);
        profilerStats.totalLoop = profilerStats.totalLED = 0;
        profilerStats.count = 0;
        profilerStats.maxLoop = profilerStats.maxLED = 0;
        profilerStats.lastPrint = now;
    }
}
#else
inline void profilerAddLoop(uint32_t) {}
inline void profilerAddLed(uint32_t) {}
inline void profilerMaybePrint() {}
#endif

#endif // PROFILER_H
