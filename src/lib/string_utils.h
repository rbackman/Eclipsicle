#pragma once
#include <string>
#include <algorithm>
#include <cctype>
inline void trim(std::string &s)
{
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        s.clear();
        return;
    }
    s = s.substr(start, end - start + 1);
}
inline void toLowerCase(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                   { return std::tolower(c); });
}

inline bool startsWith(const std::string &str, const std::string &prefix)
{
    return str.rfind(prefix, 0) == 0;
}
inline bool endsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}
inline bool contains(const std::string &str, const std::string &substr)
{
    return str.find(substr) != std::string::npos;
}
inline void replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}
inline bool equalsIgnoreCase(const std::string &a, const std::string &b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
            return false;
    }
    return true;
}
inline void toUpperCase(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                   { return std::toupper(c); });
}
inline int indexOf(const std::string &s, char c, int pos = 0)
{
    auto p = s.find(c, pos);
    return p == std::string::npos ? -1 : (int)p;
}
inline int indexOf(const std::string &s, const std::string &sub, int pos = 0)
{
    auto p = s.find(sub, pos);
    return p == std::string::npos ? -1 : (int)p;
}
inline std::string substring(const std::string &s, int start, int len = -1) { return len < 0 ? s.substr(start) : s.substr(start, len); }
inline int toInt(const std::string &s)
{
    try
    {
        return std::stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}
inline float toFloat(const std::string &s)
{
    try
    {
        return std::stof(s);
    }
    catch (...)
    {
        return 0.0f;
    }
}
