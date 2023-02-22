#pragma once

#include <cstdint>
#include <string>

#include <opencv2/opencv.hpp>
#include "SimComponentBase.hpp"

class Renderer : public SimComponentBase
{
public:
    Renderer(class Simulator* simulator, std::string dest_ip);
    ~Renderer();

    void update();

private:
    /// Chip simulator window name
    std::string sim_chip_window_ = "Chip Simulator";

    /// Marble simulator window name
    std::string sim_marble_window_ = "Marble Simulator";

    /// Chip simulator texture
    cv::Mat sim_chip_img_;

    /// Marble simulator texture
    cv::Mat sim_marble_img_;

public:
    /// Pixel size in simulator
    uint32_t pixel_size_;

    /// Destination ip address for mouse event
    std::string dest_ip_;
};
