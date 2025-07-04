#include "config.h"

void ConfigManager::begin() {
    prefs.begin("eclipsicle", false);
}

void ConfigManager::saveParameters(ParameterManager* pm) {
    if (!pm) return;
    auto name = pm->getName();
    for (const auto &p : pm->getIntParameters()) {
        String key = makeKey(name.c_str(), 'i', p.id);
        prefs.putInt(key.c_str(), p.value);
    }
    for (const auto &p : pm->getBoolParameters()) {
        String key = makeKey(name.c_str(), 'b', p.id);
        prefs.putBool(key.c_str(), p.value);
    }
    for (const auto &p : pm->getFloatParameters()) {
        String key = makeKey(name.c_str(), 'f', p.id);
        prefs.putFloat(key.c_str(), p.value);
    }
}

void ConfigManager::loadParameters(ParameterManager* pm) {
    if (!pm) return;
    auto name = pm->getName();
    for (const auto &p : pm->getIntParameters()) {
        String key = makeKey(name.c_str(), 'i', p.id);
        int val = prefs.getInt(key.c_str(), p.value);
        pm->setInt(p.id, val);
    }
    for (const auto &p : pm->getBoolParameters()) {
        String key = makeKey(name.c_str(), 'b', p.id);
        bool val = prefs.getBool(key.c_str(), p.value);
        pm->setBool(p.id, val);
    }
    for (const auto &p : pm->getFloatParameters()) {
        String key = makeKey(name.c_str(), 'f', p.id);
        float val = prefs.getFloat(key.c_str(), p.value);
        pm->setFloat(p.id, val);
    }
}

String ConfigManager::makeKey(const char* name, char type, int id) {
    String base = String(name);
    if (base.length() > 8) {
        base = base.substring(0, 8);
    }
    String key = base + "_" + String(type) + "_" + String(id);
    return key;
}

void ConfigManager::clear() {
    prefs.clear();
}
