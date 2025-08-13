#include "shared.h"

#include <algorithm>
#include <cmath>

bool _verbose = false;

std::string getName()
{
#ifdef SLAVE_NAME
  return SLAVE_NAME;
#else
  return "UNKNOWN_NAME";
#endif
}

int lerp(int fromMin, int fromMax, int toMin, int toMax, int value)
{
  return toMin + (value - fromMin) * (toMax - toMin) / (fromMax - fromMin);
}
int interpolate(int from, int to, float t)
{
  return from + (to - from) * t;
}

std::string getSensorName(SensorID type)
{
  auto it = sensorIDMap.find(type);
  if (it != sensorIDMap.end())
  {
    return it->second;
  }
  return "UNKNOWN";
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
namespace
{
  struct CHSV
  {
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
  };

  inline uint8_t scale8(uint8_t i, uint8_t scale)
  {
    return (uint16_t(i) * uint16_t(scale)) >> 8;
  }

  void hsv2rgb_raw(const CHSV &hsv, led &rgb)
  {
    uint8_t value = hsv.val;
    uint8_t saturation = hsv.sat;
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = (value * invsat) >> 8;
    uint8_t color_amplitude = value - brightness_floor;
    uint8_t section = hsv.hue / 64;
    uint8_t offset = hsv.hue & 0x3F;
    uint8_t rampup = offset;
    uint8_t rampdown = (63 - offset);
    uint8_t rampup_amp_adj = (rampup * color_amplitude) >> 6;
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) >> 6;
    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

    if (section)
    {
      if (section == 1)
      {
        rgb.r = brightness_floor;
        rgb.g = rampdown_adj_with_floor;
        rgb.b = rampup_adj_with_floor;
      }
      else
      {
        rgb.r = rampup_adj_with_floor;
        rgb.g = brightness_floor;
        rgb.b = rampdown_adj_with_floor;
      }
    }
    else
    {
      rgb.r = rampdown_adj_with_floor;
      rgb.g = rampup_adj_with_floor;
      rgb.b = brightness_floor;
    }
  }

  void hsv2rgb_spectrum(const CHSV &hsv, led &rgb)
  {
    CHSV hsv2 = hsv;
    hsv2.hue = scale8(hsv2.hue, 191);
    hsv2rgb_raw(hsv2, rgb);
  }
}

void colorFromHSV(led &color, float h, float s, float value)
{
  h = fmodf(h, 1.0f);
  if (h < 0)
    h += 1.0f;

  CHSV hsv;
  hsv.hue = uint8_t(h * 255.0f + 0.5f);
  hsv.sat = uint8_t(std::max(0.0f, std::min(1.0f, s)) * 255.0f + 0.5f);
  hsv.val = uint8_t(std::max(0.0f, std::min(1.0f, value)) * 255.0f + 0.5f);

  hsv2rgb_spectrum(hsv, color);
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
