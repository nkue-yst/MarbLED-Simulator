#pragma once

#include <cstdint>
#include <mutex>
#include <thread>
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

    void runRecvSocket();

    void updateColorData();

    bool getQuitFlag() const { return this->quit_flag_; }
    uint32_t getLedWidth() const { return this->led_width_; }
    uint32_t getLedHeight() const { return this->led_height_; }
    std::vector<std::vector<Color>> getColors() const { return this->color_mat_; }

    void setLedWidth(const uint32_t width) { this->led_width_ = width; }
    void setLedHeight(const uint32_t height) { this->led_height_ = height; }

    /// Mutex for color data
    std::mutex mutex_color_mat_;

private:
    /// Quit flag
    bool quit_flag_;

    /// Renderer component
    class Renderer* renderer_;

    /// Socket component
    class Socket* socket_;

    /// MatrixLED width
    uint32_t led_width_;

    /// MatrixLED height;
    uint32_t led_height_;

    /// Matrix LED color data
    std::vector<std::vector<Color>> color_mat_;
};
