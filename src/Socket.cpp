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
        uint32_t id = 1;
        for (int y = 0; y < h; ++y)
        {
            for (int x = w - 1; x >= 0; --x)
            {
                char filter[256] = {};
                std::sprintf(filter, "BRD_COLOR %d", id++);
                sub.setsockopt(ZMQ_SUBSCRIBE, filter, std::strlen(filter));

                msgs.clear();
                zmq::recv_multipart(sub, std::back_inserter(msgs), zmq::recv_flags::none);

                //////////////////////////////////////////////////////////////////////////////////////////
                ///// 受信したデータをColor配列にまとめてから "this->getParent()->color_mat_" に格納 /////
                //////////////////////////////////////////////////////////////////////////////////////////
                std::vector<Color> colors;
                colors.resize(PANEL_WIDTH * PANEL_HEIGHT);

                for (int j = 0; j < PANEL_WIDTH * PANEL_HEIGHT; ++j)
                {
                    colors.at(j).r = msgs.at(1).data<uint8_t>()[j];
                    colors.at(j).g = msgs.at(2).data<uint8_t>()[j];
                    colors.at(j).b = msgs.at(3).data<uint8_t>()[j];
                }

                // 応急処置
                int X, Y;
                if (id == 2)
                {
                    X = 0; Y = 0;
                }
                else if (id == 3)
                {
                    X = 1; Y = 1;
                }
                else if (id == 4)
                {
                    X = 0; Y = 1;
                }
                else if (id == 5)
                {
                    X = 1; Y = 0;
                }

                for (int j = 0; j < PANEL_WIDTH * PANEL_HEIGHT; ++j)
                {
                    uint32_t pixel_index = (Y * PANEL_HEIGHT + j / PANEL_WIDTH) * this->getParent()->getLedWidth() + (X * PANEL_WIDTH + j % PANEL_WIDTH);

                    this->getParent()->color_mat_.at(pixel_index) = colors.at(j);
                }

                /*************************
                for (int j = 0; j < PANEL_WIDTH * PANEL_HEIGHT; ++j)
                {
                    uint32_t pixel_index = (y * PANEL_HEIGHT + j / PANEL_WIDTH) * this->getParent()->getLedWidth() + (x * PANEL_WIDTH + j % PANEL_WIDTH);

                    this->getParent()->color_mat_.at(pixel_index) = colors.at(j);
                }
                **************************/
            }
        }
    }

    sub.close();
}
