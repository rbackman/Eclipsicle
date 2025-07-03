#include "shared.h"
#include <WiFi.h>

bool _verbose = false;
String _name = "default";
String getName()
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
// std::vector<ParameterID> getParametersForMenu(MenuID menu)
// {
//   std::vector<ParameterID> menuParams = {};
//   for (auto it = parameterMenuList.begin(); it != parameterMenuList.end(); ++it)
//   {
//     if (it->first == menu)
//     {
//       menuParams.push_back(it->second);
//     }
//   }
//   return menuParams;
// }
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
  String path = getMenuName(type);
  MenuID parent = getParentMenu(type);
  while (parent != MENU_ROOT && parent != root)
  {
    path = String(getMenuName(parent)) + "/" + String(path.c_str());
    parent = getParentMenu(parent);
  }
  return path.c_str();
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

void printBytes(ByteRow data)
{
  Serial.println("bytes -->");
  for (int i = 0; i < data.size(); ++i)
  {
    Serial.printf("%d ", data[i]);
    // if ((i + 1) % 16 == 0)
    //   Serial.println();
  }
  Serial.println("<-- as bytes");
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

void colorFromHSV(led &color, float h, float s, float v)
{
  float r, g, b;
  int i = int(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  switch (i % 6)
  {
  case 0:
    r = v, g = t, b = p;
    break;
  case 1:
    r = q, g = v, b = p;
    break;
  case 2:
    r = p, g = v, b = t;
    break;
  case 3:
    r = p, g = q, b = v;
    break;
  case 4:
    r = t, g = p, b = v;
    break;
  case 5:
    r = v, g = p, b = q;
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

std::vector<String> splitString(const String &path, char delimiter)
{
  std::vector<String> result;
  size_t start = 0;
  size_t end = path.indexOf(delimiter);
  while (end != -1)
  {
    result.push_back(path.substring(start, end));
    start = end + 1;
    end = path.indexOf(delimiter, start);
  }
  result.push_back(path.substring(start));
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
      Serial.printf("Parameter %s not found in default parameters\n", names[i].c_str());
    }
  }
  if (missingParams == 0)
  {
    Serial.println("All parameters accounted for");
  }
}