#include "uart_manager.h"
#include "shared.h"

UARTManager *UARTManager::instance = nullptr;

#ifndef SLAVE_RX
#define SLAVE_RX 37
#endif

#ifndef SLAVE_TX
#define SLAVE_TX 38
#endif

void UARTManager::beginMaster() { instance = this; }

void UARTManager::beginSlave(uint8_t addr, UARTMessageHandler handler) {
    _address = addr;
    _serial = new HardwareSerial(1);
    _serial->begin(115200, SERIAL_8N1, SLAVE_RX, SLAVE_TX);
    _serial->setTimeout(0);
    _handler = handler;
    instance = this;
}

void UARTManager::addSlave(uint8_t address, int8_t rxPin, int8_t txPin,
                           uint8_t uartNum) {
    HardwareSerial *port = new HardwareSerial(uartNum);
    port->begin(115200, SERIAL_8N1, rxPin, txPin);
    port->setTimeout(0);
    _slaves.push_back({address, port, ""});
}

void UARTManager::sendString(uint8_t address, const std::string &message) {
    for (auto &s : _slaves) {
        if (s.address == address) {
            s.serial->write(reinterpret_cast<const uint8_t *>(message.c_str()),
                            message.length());
            s.serial->write('\n');
            break;
        }
    }
}

void UARTManager::broadcastString(const std::string &message, bool print) {
    if (isVerbose() && print) {
        Serial.print("Broadcasting UART message: ");
        Serial.println(message.c_str());
    }
    for (auto &s : _slaves) {
        s.serial->write(reinterpret_cast<const uint8_t *>(message.c_str()),
                        message.length());
        s.serial->write('\n');
    }
}

bool UARTManager::ping(uint8_t address) {
    sendString(address, "PING");
    return true;
}

void UARTManager::testSlaves() {
    for (auto &s : _slaves) {
        bool ok = ping(s.address);
        Serial.print("UART slave 0x");
        Serial.print(s.address, HEX);
        if (ok) {
            Serial.println(" responded");
        } else {
            Serial.println(" not responding");
        }
    }
}

void UARTManager::sendSync(uint32_t timeMs) {
    std::string msg = "SYNC:" + std::to_string(timeMs);
    broadcastString(msg, false);
}

void UARTManager::update() {
    if (_serial) {
        while (_serial->available()) {
            char c = _serial->read();
            if (c == '\n') {
                if (_handler && !_rxBuffer.empty()) {
                    _handler(_rxBuffer);
                }
                _rxBuffer.clear();
            } else if (c != '\r') {
                _rxBuffer += c;
            }
        }
    }

    for (auto &s : _slaves) {
        while (s.serial->available()) {
            char c = s.serial->read();
            if (c == '\n') {
                if (_handler && !s.buffer.empty()) {
                    _handler(s.buffer);
                }
                s.buffer.clear();
            } else if (c != '\r') {
                s.buffer += c;
            }
        }
    }
}

