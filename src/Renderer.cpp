#include <algorithm>

#include "Renderer.hpp"
#include "Color.hpp"
#include "Common.hpp"
#include "Simulator.hpp"

Renderer::Renderer(Simulator* simulator)
    : SimComponentBase(simulator)
{
    // Display size settings
    uint32_t display_width = 1920;
    uint32_t display_height = 1080;

    // Pixel size settings
    this->pixel_size_ = std::min(display_width / this->getParent()->getLedWidth(), display_height / this->getParent()->getLedHeight()) / 2;
    this->blank_size_ = this->pixel_size_ / 3;

    this->update();

    printLog("Init Renderer", true);
}

Renderer::~Renderer()
{
    printLog("Destroy Renderer", true);
}

void Renderer::update()
{
    // Set background mat
    this->sim_chip_img_ = cv::Mat(
        this->pixel_size_ * this->getParent()->getLedHeight(),
        this->pixel_size_ * this->getParent()->getLedWidth(),
        CV_8UC3,
        cv::Scalar(90, 90, 90)
    );

    // Draw chips
    for (uint32_t y = 0; y < this->getParent()->getLedHeight(); y++)
    {
        for (uint32_t x = 0; x < this->getParent()->getLedWidth(); x++)
        {
            Color p_color = this->getParent()->getColor(x, y);

            if (!(p_color.r == 0 && p_color.g == 0 && p_color.b == 0))
            {
                cv::circle(
                    this->sim_chip_img_,
                    cv::Point(
                        this->pixel_size_ * x + this->pixel_size_ / 2,
                        this->pixel_size_ * y + this->pixel_size_ / 2
                    ),
                    static_cast<int32_t>(this->pixel_size_ / 2 + this->pixel_size_ * 0.2f),
                    cv::Scalar(
                        p_color.b,
                        p_color.g,
                        p_color.r
                    ),
                    -1
                );
            }
        }
    }

    // Create marble sim
    cv::GaussianBlur(
        this->sim_chip_img_,
        this->sim_marble_img_,
        cv::Size(51, 51),
        0
    );

    // Show windows
    cv::imshow(this->sim_chip_window_, this->sim_chip_img_);
    cv::imshow(this->sim_marble_window_, this->sim_marble_img_);
    cv::waitKey(16);
}