#include "shared.h"
#include <WiFi.h>

bool _verbose = true;
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
const char *getParameterName(ParameterID type)
{
  auto it = parameterMenuMap.find(type);
  if (it != parameterMenuMap.end())
  {
    return it->second.name.c_str();
  }
  return "UNKNOWN";
}
const char *getMenuName(MenuID type)
{
  auto it = menuTypeMap.find(type);
  if (it != menuTypeMap.end())
  {
    return it->second.first.c_str();
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

  // if (h != 0)
  // {
  //   printf("rgb :%d %d %d :", color.r, color.g, color.b);
  // }
}
