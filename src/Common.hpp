#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

inline void printLog(const char *action, bool success)
{
    std::cout << (success ? "[\033[1;32mOK\033[m]: " : "[\033[1;31mNG\033[m]: ");
    std::cout << action << std::endl;
}

inline void delay(const uint32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif
