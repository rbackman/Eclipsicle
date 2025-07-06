#ifndef ARDUINOJSON_STUB_H
#define ARDUINOJSON_STUB_H
#include <string>
#include <vector>
#include <map>

class JsonVariant {
public:
    template<typename T> bool is() const { return false; }
    template<typename T> T as() const { return T(); }
    template<typename T> T to() const { return T(); }
    JsonVariant& operator=(int) { return *this; }
    JsonVariant& operator=(float) { return *this; }
    JsonVariant& operator=(bool) { return *this; }
    JsonVariant& operator=(const char*) { return *this; }
    JsonVariant& operator=(const std::string&) { return *this; }
};

class JsonObject {
    std::map<std::string, JsonVariant> data;
public:
    JsonVariant& operator[](const std::string& key) { return data[key]; }
    template<typename T> T to() { return T(); }
};

class JsonArray : public std::vector<JsonVariant> {
public:
    template<typename T> T to() { return T(); }
    template<typename T = JsonVariant> T add() { this->emplace_back(); return T(); }
};

class JsonDocument : public JsonObject {};

template<typename T>
void serializeJson(const JsonDocument&, T&) {}

#endif // ARDUINOJSON_STUB_H
