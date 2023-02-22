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

    void run(std::string dest_ip = "127.0.0.1");
    bool update();

    void runRecvSocket(std::string dest_ip);

    void updateColorData();

    bool getQuitFlag() const { return this->quit_flag_; }
    uint32_t getLedWidth() const { return this->led_width_; }
    uint32_t getLedHeight() const { return this->led_height_; }
    std::vector<Color> getColors() const { return this->color_mat_; }

    void setQuitFlag(const bool flag) { this->quit_flag_ = flag; }
    void setLedWidth(const uint32_t width) { this->led_width_ = width; }
    void setLedHeight(const uint32_t height) { this->led_height_ = height; }

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

public:
    /// Matrix LED color data
    std::vector<Color> color_mat_;
};
