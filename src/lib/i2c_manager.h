#pragma once

#include <Wire.h>
#include <vector>
#include <functional>
#include <string>

using I2CMessageHandler = std::function<void(const std::string &)>;

class I2CManager {
  public:
    void beginMaster();
    void beginSlave(uint8_t address, I2CMessageHandler handler);
    void addSlave(uint8_t address);
    void sendString(uint8_t address, const std::string &message);
    void broadcastString(const std::string &message);
    bool ping(uint8_t address);
    void testSlaves();
    void sendSync(uint32_t timeMs);

  private:
    static I2CManager *instance;
    I2CMessageHandler _handler;
    std::vector<uint8_t> _slaves;

    static void onReceiveStatic(int numBytes);
    void onReceive(int numBytes);
};

