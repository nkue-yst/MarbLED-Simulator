#pragma once

#include <cstdint>
#include <string>

#include <opencv2/opencv.hpp>
#include "SimComponentBase.hpp"

class Renderer : public SimComponentBase
{
public:
    Renderer(class Simulator* simulator);
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

    /// Pixel size in simulator
    uint32_t pixel_size_;

    /// Blank size per pixel
    uint32_t blank_size_;
};
