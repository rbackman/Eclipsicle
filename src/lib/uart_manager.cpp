#include "uart_manager.h"
#include "shared.h"

UARTManager *UARTManager::instance = nullptr;

#ifndef SLAVE_RX
#define SLAVE_RX 37
#endif

#ifndef SLAVE_TX
#define SLAVE_TX 38
#endif

static HardwareSerial UART_PORT(1);

void UARTManager::beginMaster() {
    UART_PORT.begin(115200, SERIAL_8N1, SLAVE_RX, SLAVE_TX);
    instance = this;
}

void UARTManager::beginSlave(uint8_t addr, UARTMessageHandler handler) {
    _address = addr;
    UART_PORT.begin(115200, SERIAL_8N1, SLAVE_RX, SLAVE_TX);
    _handler = handler;
    instance = this;
}

void UARTManager::addSlave(uint8_t address) { _slaves.push_back(address); }

void UARTManager::sendString(uint8_t address, const std::string &message) {
    UART_PORT.write(address);
    UART_PORT.write(reinterpret_cast<const uint8_t *>(message.c_str()),
                    message.length());
    UART_PORT.write('\n');
}

void UARTManager::broadcastString(const std::string &message, bool print) {
    if (isVerbose() && print) {
        Serial.print("Broadcasting UART message: ");
        Serial.println(message.c_str());
    }
    for (auto addr : _slaves) {
        sendString(addr, message);
    }
}

bool UARTManager::ping(uint8_t address) {
    sendString(address, "PING");
    return true;
}

void UARTManager::testSlaves() {
    for (auto addr : _slaves) {
        bool ok = ping(addr);
        Serial.print("UART slave 0x");
        Serial.print(addr, HEX);
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
    while (UART_PORT.available()) {
        uint8_t addr = UART_PORT.read();
        std::string msg;
        while (UART_PORT.available()) {
            char c = UART_PORT.read();
            if (c == '\n')
                break;
            msg.push_back(c);
        }
        if (_handler && (addr == _address || addr == 0xFF)) {
            _handler(msg);
        }
    }
}

