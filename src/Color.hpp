#pragma once

#include <cstdint>

struct Color
{
public:
    Color(uint32_t red = 0, uint32_t green = 0, uint32_t blue = 0)
        : r(red)
        , g(green)
        , b(blue)
    {
    }

    uint32_t r, g, b;
};
