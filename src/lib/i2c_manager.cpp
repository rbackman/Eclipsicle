#include "i2c_manager.h"

I2CManager *I2CManager::instance = nullptr;

void I2CManager::beginMaster() {
    Wire.begin();
    instance = this;
}

void I2CManager::beginSlave(uint8_t address, I2CMessageHandler handler) {
    Wire.begin(address);
    _handler = handler;
    instance = this;
    Wire.onReceive(I2CManager::onReceiveStatic);
}

void I2CManager::addSlave(uint8_t address) { _slaves.push_back(address); }

void I2CManager::sendString(uint8_t address, const std::string &message) {
    Wire.beginTransmission(address);
    Wire.write(reinterpret_cast<const uint8_t *>(message.c_str()), message.length());
    Wire.endTransmission();
}

void I2CManager::broadcastString(const std::string &message) {
    for (auto addr : _slaves) {
        sendString(addr, message);
    }
}

void I2CManager::sendSync(uint32_t timeMs) {
    std::string msg = "SYNC:" + std::to_string(timeMs);
    broadcastString(msg);
}

void I2CManager::onReceiveStatic(int numBytes) {
    if (instance) {
        instance->onReceive(numBytes);
    }
}

void I2CManager::onReceive(int numBytes) {
    if (!_handler)
        return;
    std::string msg;
    msg.reserve(numBytes);
    while (Wire.available()) {
        char c = Wire.read();
        msg.push_back(c);
    }
    _handler(msg);
}

