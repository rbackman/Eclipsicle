#include "shared.h"

#include <cmath>

bool _verbose = false;
std::string _name = "default";

std::vector<LEDRig> ledRigs = {

};

std::vector<LEDRig> getLEDRigs()
{
  return ledRigs;
}
LEDRig *getLEDRig(const std::string &name)
{
  for (auto &rig : ledRigs)
  {
    if (rig.name == name)
    {
      return &rig;
    }
  }
  return nullptr;
}

std::string getName()
{
  return _name;
}

int lerp(int fromMin, int fromMax, int toMin, int toMax, int value)
{
  return toMin + (value - fromMin) * (toMax - toMin) / (fromMax - fromMin);
}
int interpolate(int from, int to, float t)
{
  return from + (to - from) * t;
}
const char *getMenuName(MenuID type, int MaxSize)
{
  auto it = menuTypeMap.find(type);
  if (it != menuTypeMap.end())
  {
    auto name = it->second.first;
    if (name.size() > MaxSize)
    {
      return name.substr(0, MaxSize).c_str();
    }
    return name.c_str();
  }
  return "UNKNOWN";
}

const char *getSensorName(SensorID type)
{
  auto it = sensorIDMap.find(type);
  if (it != sensorIDMap.end())
  {
    return it->second.c_str();
  }
  return "UNKNOWN";
}

std::vector<MenuID> getChildrenOfMenu(MenuID type)
{
  std::vector<MenuID> children;
  for (auto it = menuTypeMap.begin(); it != menuTypeMap.end(); ++it)
  {
    if (it->second.second == type)
    {
      children.push_back(it->first);
    }
  }
  return children;
}
std::string getMenuPath(MenuID type, MenuID root)
{
  std::string path = getMenuName(type);
  MenuID parent = getParentMenu(type);
  while (parent != MENU_ROOT && parent != root)
  {
    path = std::string(getMenuName(parent)) + "/" + path;
    parent = getParentMenu(parent);
  }
  return path;
}
const MenuID getParentMenu(MenuID type)
{
  auto it = menuTypeMap.find(type);
  if (it != menuTypeMap.end())
  {
    return it->second.second;
  }
  return MENU_IDLE;
}

void setVerbose(bool verb)
{
  _verbose = verb;
}
bool isVerbose()
{
  return _verbose;
}

bool isSlider(SensorID id)
{
  for (int i = 0; i < sizeof(sliderIDs) / sizeof(sliderIDs[0]); i++)
  {
    if (id == sliderIDs[i])
    {
      return true;
    }
  }
  return false;
}
void colorFromHSV(led &color, float h, float s, float value)
{
  // Clamp or wrap hue to [0, 1)
  h = fmodf(h, 1.0f);
  if (h < 0)
    h += 1.0f;

  float r, g, b;
  int i = int(h * 6);
  float f = h * 6 - i;
  float p = value * (1 - s);
  float q = value * (1 - f * s);
  float t = value * (1 - (1 - f) * s);

  switch (i % 6)
  {
  case 0:
    r = value, g = t, b = p;
    break;
  case 1:
    r = q, g = value, b = p;
    break;
  case 2:
    r = p, g = value, b = t;
    break;
  case 3:
    r = p, g = q, b = value;
    break;
  case 4:
    r = t, g = p, b = value;
    break;
  case 5:
    r = value, g = p, b = q;
    break;
  }

  color.r = int(r * 255);
  color.g = int(g * 255);
  color.b = int(b * 255);
}

std::vector<std::string> getParameterNames()
{
  std::vector<std::string> names = {
#define X(name) #name,
      PARAMETER_LIST
#undef X
  };
  return names;
}
std::string getParameterName(ParameterID type)
{
  auto names = getParameterNames();
  if (type < 0 || type >= names.size())
  {
    return "UNKNOWN";
  }

  std::string name = names[type];
  name.erase(0, 6);
  return name;
}
std::vector<std::string> splitString(const std::string &path, char delimiter)
{
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = path.find(delimiter);
  while (end != std::string::npos)
  {
    result.push_back(path.substr(start, end - start));
    start = end + 1;
    end = path.find(delimiter, start);
  }
  result.push_back(path.substr(start)); // push the last segment
  return result;
}

std::vector<std::string> getAnimationNames()
{
  std::vector<std::string> names = {
#define X(name) #name,
      ANIMATION_LIST
#undef X
  };
  return names;
}
std::string getAnimationName(ANIMATION_TYPE type)
{
  auto names = getAnimationNames();
  if (type < 0 || type >= names.size())
  {
    return "UNKNOWN";
  }
  // return names[type];
  // remove ANIMATION_TYPE_ prefix
  std::string name = names[type];
  name.erase(0, 15);
  return name;
}

bool isBoolParameter(ParameterID id)
{
  auto boolparams = getDefaultBoolParameters();
  for (int i = 0; i < boolparams.size(); i++)
  {
    if (boolparams[i].id == id)
    {
      return true;
    }
  }
  return false;
}

bool isFloatParameter(ParameterID id)
{
  auto floatparams = getDefaultFloatParameters();
  for (int i = 0; i < floatparams.size(); i++)
  {
    if (floatparams[i].id == id)
    {
      return true;
    }
  }
  return false;
}
bool isIntParameter(ParameterID id)
{
  auto intparams = getDefaultIntParameters();
  for (int i = 0; i < intparams.size(); i++)
  {
    if (intparams[i].id == id)
    {
      return true;
    }
  }
  return false;
}

void sanityCheckParameters()
{
  auto names = getParameterNames();
  auto intParams = getDefaultIntParameters();
  auto boolParams = getDefaultBoolParameters();
  auto floatParams = getDefaultFloatParameters();
  int missingParams = 0;
  for (int i = 0; i < names.size(); i++)
  {
    // i is equivilent to the parameter ID
    // search for i as a ParameterID in the intParams and boolParams
    bool found = false;
    for (auto param : intParams)
    {
      if (param.id == i)
      {
        found = true;
        break;
      }
    }
    if (found)
    {
      continue;
    }
    for (auto param : boolParams)
    {
      if (param.id == i)
      {
        found = true;
        break;
      }
    }
    if (found)
    {
      continue;
    }
    for (auto param : floatParams)
    {
      if (param.id == i)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      missingParams++;
      printf("Missing parameter ID %d (%s)\n", i, names[i].c_str());
    }
  }
  if (missingParams == 0)
  {
    printf("All parameters accounted for\n");
  }
}

void makeRig(const std::string &name, const MacAddress &mac)
{
  LEDRig rig;
  rig.name = name;
  rig.mac = mac;
  ledRigs.push_back(rig);
}

void addStripToRig(const std::string &name, int stripIndex, int pin, int numLEDS, LED_STATE state, std::vector<AnimationParams> animations, std::vector<Node3D> nodes)
{
  for (auto &rig : ledRigs)
  {
    if (rig.name == name)
    {
      LEDParams params;
      params.stripIndex = stripIndex;
      params.pin = pin;
      params.numLEDS = numLEDS;
      params.state = state;
      params.animations = animations;
      params.nodes = nodes;

      rig.strips.push_back(params);
      return;
    }
  }
}
