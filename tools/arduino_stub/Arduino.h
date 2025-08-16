#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H
#include <string>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdint>

#define HIGH 1
#define LOW 0

class String : public std::string {
public:
    using std::string::string;
    String() : std::string() {}
    String(const std::string& s) : std::string(s) {}
    String(const char* s) : std::string(s) {}
    String(int v) : std::string(std::to_string(v)) {}
    String(float v) : std::string(std::to_string(v)) {}
    String(double v) : std::string(std::to_string(v)) {}
    String substring(size_t pos, size_t len=npos) const { return String(std::string::substr(pos,len)); }
    int indexOf(char c, size_t pos=0) const { auto p=this->find(c,pos); return p==npos?-1:(int)p; }
    int indexOf(const String &s, size_t pos=0) const { auto p=this->find(s,pos); return p==npos?-1:(int)p; }
    bool startsWith(const String &prefix) const { return this->rfind(prefix,0)==0; }
    bool endsWith(const String &suffix) const { return this->size()>=suffix.size() && this->compare(this->size()-suffix.size(), suffix.size(), suffix)==0; }
    void trim() { auto start=this->find_first_not_of(" \t\r\n"); auto end=this->find_last_not_of(" \t\r\n"); if(start==npos){ this->clear(); } else { *this = this->substr(start, end-start+1); } }
    void toUpperCase() { for(auto &c:*this) c=std::toupper((unsigned char)c); }
    bool equalsIgnoreCase(const String& s) const { if(size()!=s.size()) return false; for(size_t i=0;i<size();++i) if(std::tolower((*this)[i])!=std::tolower(s[i])) return false; return true; }
    bool equals(const String& s) const { return *this == s; }
    int toInt() const { return std::stoi(*this); }
    float toFloat() const { return std::stof(*this); }
    void replace(char from, char to) { for(auto &c:*this) if(c==from) c=to; }
    size_t write(const char* s) { this->append(s); return std::strlen(s); }
    size_t write(const uint8_t* s, size_t n) { this->append((const char*)s,n); return n; }
    size_t write(uint8_t c) { this->push_back(c); return 1; }
    using std::string::operator=;
};

struct SerialClass {
    template<typename... Args>
    void printf(const char *fmt, Args... args) { std::printf(fmt, args...); }
    void println(const String &s) { std::printf("%s\n", s.c_str()); }
    void println() { std::printf("\n"); }
    void print(const String &s) { std::printf("%s", s.c_str()); }
    size_t write(const char* s) { return std::fwrite(s,1,std::strlen(s),stdout); }
    size_t write(const uint8_t* s, size_t n) { return std::fwrite(s,1,n,stdout); }
    size_t write(uint8_t c) { return std::fwrite(&c,1,1,stdout); }
    void begin(unsigned long) {}
};
static SerialClass Serial;
inline long random(long min, long max) { return min + std::rand() % (max - min); }
inline void delay(unsigned long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}
template<typename T>
inline T constrain(T val, T minVal, T maxVal) {
    return std::min(std::max(val, minVal), maxVal);
}

// minimal FastLED noise stubs
inline uint8_t inoise8(uint16_t x) { return (uint8_t)(std::rand() % 256); }
inline uint8_t inoise8(uint16_t x, uint16_t y) { return (uint8_t)(std::rand() % 256); }

struct CRGB;

inline String getLedStateName(int state) {
    switch(state) {
        case 0: return String("IDLE");
        case 1: return String("SINGLEANIMATION");
        case 2: return String("MULTIANIMATION");
        case 3: return String("POINTCONTROL");
    }
    return String("UNKNOWN");
}
#endif
