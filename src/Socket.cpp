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
        uint32_t i = 0;
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                char filter[256] = {};
                std::sprintf(filter, "BRD_COLOR %d", i++);
                sub.setsockopt(ZMQ_SUBSCRIBE, filter, std::strlen(filter));

                // Debug print for filter
                // std::cout << filter << std::endl;

                zmq::recv_multipart(sub, std::back_inserter(msgs), zmq::recv_flags::none);

                // Debug print for received data
                // std::cout << msgs[0].to_string() << std::endl;
                // std::cout << msgs[1].to_string() << std::endl;
                // std::cout << msgs[2].to_string() << std::endl;
                // std::cout << msgs[3].to_string() << std::endl;
            }
        }
    }

    sub.close();
}
