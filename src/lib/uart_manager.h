#pragma once

#include <Arduino.h>
#include <functional>
#include <string>
#include <vector>

using UARTMessageHandler = std::function<void(const std::string &)>;

class UARTManager {
  public:
    void beginMaster();
    void beginSlave(uint8_t address, UARTMessageHandler handler);
    void addSlave(uint8_t address);
    void sendString(uint8_t address, const std::string &message);
    void broadcastString(const std::string &message, bool print = true);
    bool ping(uint8_t address);
    void testSlaves();
    void sendSync(uint32_t timeMs);
    void update();

  private:
    static UARTManager *instance;
    UARTMessageHandler _handler;
    std::vector<uint8_t> _slaves;
    uint8_t _address = 0;
};

