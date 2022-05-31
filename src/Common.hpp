#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

inline void printLog(const char *action, bool success)
{
    std::cout << (success ? "[OK]: " : "[NG]: ");
    std::cout << action << std::endl;
}

inline void delay(const uint32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
