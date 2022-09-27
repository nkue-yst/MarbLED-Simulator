#include <algorithm>

#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"
#include "ip/UdpSocket.h"

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

    this->update();

    printLog("Init Renderer", true);
}

Renderer::~Renderer()
{
    printLog("Destroy Renderer", true);
}

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    UdpTransmitSocket sock(IpEndpointName("127.0.0.1", 9000));

    char buff[1024];
    osc::OutboundPacketStream p(buff, 1024);

    Renderer* renderer = static_cast<Renderer*>(userdata);

    // マウス座標をパネル上での座標に変換する
    int32_t pos_x = x / renderer->pixel_size_;
    int32_t pos_y = y / renderer->pixel_size_;

    static bool l_down = false;

    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        l_down = true;
    case cv::EVENT_MOUSEMOVE:
        if (l_down)
        {
            p << osc::BeginBundleImmediate
                << osc::BeginMessage("/touch/0/point")
                    << pos_x
                    << pos_y
                << osc::EndMessage
            << osc::EndBundle;
            sock.Send(p.Data(), p.Size());
        }
        break;

    case cv::EVENT_LBUTTONUP:
        l_down = false;
        p << osc::BeginBundleImmediate
            << osc::BeginMessage("/touch/0/delete")
            << osc::EndMessage
        << osc::EndBundle;
        sock.Send(p.Data(), p.Size());
        break;

    default:
        break;
    }

    //std::cout << "x: " << pos_x << "\n" << "y: " << pos_y << std::endl;
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
    std::vector<Color> color_mat = this->getParent()->getColors();

    this->getParent()->mutex_color_mat_.lock();

    for (uint32_t y = 0; y < this->getParent()->getLedHeight(); y++)
    {
        for (uint32_t x = 0; x < this->getParent()->getLedWidth(); x++)
        {
            Color p_color = color_mat[x + this->getParent()->getLedWidth() * y];

            if (!(p_color.r == 0 && p_color.g == 0 && p_color.b == 0))
            {
                cv::circle(
                    this->sim_chip_img_,
                    cv::Point(
                        this->pixel_size_ * x + this->pixel_size_ / 2,
                        this->pixel_size_ * y + this->pixel_size_ / 2
                    ),
                    static_cast<int32_t>(this->pixel_size_ / 2),
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

    this->getParent()->mutex_color_mat_.unlock();

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

    cv::setMouseCallback(this->sim_chip_window_, mouseCallback, this);
    cv::setMouseCallback(this->sim_marble_window_, mouseCallback, this);

    cv::waitKey(66);
}
