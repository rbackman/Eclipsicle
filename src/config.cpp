#include "config.h"

void ConfigManager::begin() {
    prefs.begin("eclipsicle", false);
}

void ConfigManager::saveParameters(ParameterManager* pm) {
    if (!pm) return;
    auto name = pm->getName();
    for (const auto &p : pm->getIntParameters()) {
        String key = String(name.c_str()) + "_i_" + p.id;
        prefs.putInt(key.c_str(), p.value);
    }
    for (const auto &p : pm->getBoolParameters()) {
        String key = String(name.c_str()) + "_b_" + p.id;
        prefs.putBool(key.c_str(), p.value);
    }
    for (const auto &p : pm->getFloatParameters()) {
        String key = String(name.c_str()) + "_f_" + p.id;
        prefs.putFloat(key.c_str(), p.value);
    }
}

void ConfigManager::loadParameters(ParameterManager* pm) {
    if (!pm) return;
    auto name = pm->getName();
    for (const auto &p : pm->getIntParameters()) {
        String key = String(name.c_str()) + "_i_" + p.id;
        int val = prefs.getInt(key.c_str(), p.value);
        pm->setInt(p.id, val);
    }
    for (const auto &p : pm->getBoolParameters()) {
        String key = String(name.c_str()) + "_b_" + p.id;
        bool val = prefs.getBool(key.c_str(), p.value);
        pm->setBool(p.id, val);
    }
    for (const auto &p : pm->getFloatParameters()) {
        String key = String(name.c_str()) + "_f_" + p.id;
        float val = prefs.getFloat(key.c_str(), p.value);
        pm->setFloat(p.id, val);
    }
}
