#include "i2c_manager.h"
#include "shared.h"
#include <Arduino.h>
I2CManager *I2CManager::instance = nullptr;


#ifndef SLAVE_SDA
#define SLAVE_SDA 20
#endif

#ifndef SLAVE_SCL
#define SLAVE_SCL 21
#endif

void I2CManager::beginMaster() {
    Wire.begin(SLAVE_SDA, SLAVE_SCL);
    instance = this;
}

void I2CManager::beginSlave(uint8_t address, I2CMessageHandler handler) {
    // Initialize as a slave on the given address using the custom pins.
    // `Wire.begin` needs the address parameter to enable slave mode on ESP32.
    Wire.begin(address, SLAVE_SDA, SLAVE_SCL);
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
    if(isVerbose()) {
        Serial.print("Broadcasting I2C message: ");
        Serial.println(message.c_str());
    }
    for (auto addr : _slaves) {
        sendString(addr, message);
    }
}

bool I2CManager::ping(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    return error == 0;
}

void I2CManager::testSlaves() {
    for (auto addr : _slaves) {
        bool ok = ping(addr);
        Serial.print("I2C slave 0x");
        Serial.print(addr, HEX);
        if (ok) {
            Serial.println(" responded");
        } else {
            Serial.println(" not responding");
        }
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

