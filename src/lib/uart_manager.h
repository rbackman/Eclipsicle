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
    void addSlave(uint8_t address, int8_t rxPin, int8_t txPin,
                  uint8_t uartNum);
    void sendString(uint8_t address, const std::string &message);
    void broadcastString(const std::string &message, bool print = true);
    bool ping(uint8_t address);
    void testSlaves();
    void sendSync(uint32_t timeMs);
    void update();

  private:
    struct SlaveConnection {
        uint8_t address;
        HardwareSerial *serial;
        std::string buffer;
    };

    static UARTManager *instance;
    UARTMessageHandler _handler;
    std::vector<SlaveConnection> _slaves;
    HardwareSerial *_serial = nullptr; // used in slave mode
    std::string _rxBuffer;
    uint8_t _address = 0;
};

