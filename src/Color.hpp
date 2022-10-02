#pragma once

#include <cstdint>

struct Color
{
public:
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
        : r(red)
        , g(green)
        , b(blue)
    {
    }

    uint8_t r, g, b;
};
