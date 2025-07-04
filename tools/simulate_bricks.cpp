#include "../tests/FallingBricks.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    FallingBricksSim sim(10, 2, 1.0f, false);
    for (int step=0; step<20; ++step) {
        sim.update();
        for (const auto& p : sim.pixels()) {
            std::cout << (p.r ? '#' : '.');
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
