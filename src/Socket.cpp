#include "Socket.hpp"

#include <cstring>
#include <unistd.h>
#include <vector>

#include "Common.hpp"
#include "Simulator.hpp"

#include <zmq.hpp>
#include <zmq_addon.hpp>

Socket::Socket(Simulator* simulator)
    :SimComponentBase(simulator)
{
    printLog("Create socket component", true);
}

Socket::~Socket()
{
    printLog("Destroy Socket", true);
}

void Socket::run(std::string dest_ip)
{
    /* 受信ソケットの作成 */
    zmq::context_t ctx;
    zmq::socket_t sub(ctx, zmq::socket_type::sub);

    std::string dest = "tcp://" + dest_ip + ":44100";

    sub.connect(dest);

    // パネルの縦横の枚数を取得
    int32_t w = this->getParent()->getLedWidth() / PANEL_WIDTH;
    int32_t h = this->getParent()->getLedHeight() / PANEL_HEIGHT;

    std::vector<zmq::message_t> msgs;
    while (!this->getParent()->getQuitFlag())
    {
        // パネルの縦横の枚数分のデータを受信する
        uint32_t id = 0;
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                char filter[256] = {};
                std::sprintf(filter, "BRD_COLOR %d", id++);
                sub.setsockopt(ZMQ_SUBSCRIBE, filter, std::strlen(filter));

                // Debug print for filter
                // std::cout << filter << std::endl;

                zmq::recv_multipart(sub, std::back_inserter(msgs), zmq::recv_flags::none);

                // Debug print for received data
                // std::cout << msgs[0].to_string() << std::endl;
                // std::cout << msgs[1].to_string() << std::endl;
                // std::cout << msgs[2].to_string() << std::endl;
                // std::cout << msgs[3].to_string() << std::endl;

                // 受信したデータのサイズを計算
                uint32_t array_size = msgs.at(1).size() / sizeof(uint8_t);

                ///////////////////////////////////////////////////////////////////
                ///// 受信したデータを "this->getParent()->color_mat_" に格納 /////
                ///////////////////////////////////////////////////////////////////
                std::vector<Color> colors;
                colors.resize(array_size);

                for (int j = 0; j < array_size; ++j)
                {
                    colors.at(j).r = msgs.at(1).data<uint8_t>()[j];
                    colors.at(j).g = msgs.at(2).data<uint8_t>()[j];
                    colors.at(j).b = msgs.at(3).data<uint8_t>()[j];

                    // Debug print for received data
                    std::cout <<
                        " ID: " << std::setw(3) << j <<
                        "  R: " << std::setw(3) << (int)colors.at(j).r <<
                        "  G: " << std::setw(3) << (int)colors.at(j).g <<
                        "  B: " << std::setw(3) << (int)colors.at(j).b
                    << std::endl;
                }

                for (int j = 0; j < array_size; ++j)
                {
                    uint32_t pixel_index = (y * PANEL_HEIGHT + j / PANEL_WIDTH) * this->getParent()->getLedWidth() + (x * PANEL_WIDTH + j % PANEL_WIDTH);

                    // Debug print for updated pixel coordinate
                    // std::cout << pixel_index << std::endl;

                    this->getParent()->color_mat_.at(pixel_index) = colors.at(j);
                }
            }
        }
    }

    sub.close();
}
