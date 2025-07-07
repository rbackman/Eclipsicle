#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include <Preferences.h>
#include "parameterManager.h"

class ConfigManager {
public:
    void begin();
    void saveParameters(ParameterManager* pm);
    void loadParameters(ParameterManager* pm);
    String makeKey(const char* name, char type, int id);
    void clear();
private:
    Preferences prefs;
};

#endif // CONFIG_MANAGER_H
