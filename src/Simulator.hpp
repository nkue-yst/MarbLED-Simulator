#pragma once

#include <cstdint>
#include <vector>

#include "Color.hpp"

/**
 * @brief  Simulator main class
 */
class Simulator
{
public:
    Simulator();
    ~Simulator();

    void run();
    bool update();

    uint32_t getLedWidth() const { return this->led_width_; }
    uint32_t getLedHeight() const { return this->led_height_; }
    Color getColor(uint32_t x, uint32_t y) const { return this->color_mat[y][x]; }

    void setLedWidth(const uint32_t width) { this->led_width_ = width; }
    void setLedHeight(const uint32_t height) { this->led_height_ = height; }

private:
    /// Quit flag
    bool quit_flag_;

    /// Renderer component
    class Renderer* renderer_;

    /// MatrixLED width
    uint32_t led_width_;

    /// MatrixLED height;
    uint32_t led_height_;

    /// Matrix LED color data
    std::vector<std::vector<Color>> color_mat;
};
