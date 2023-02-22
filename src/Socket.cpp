#include "Socket.hpp"

#include <cstring>
#include <unistd.h>
#include <vector>

#include "Common.hpp"
#include "Simulator.hpp"

#include <zmq.hpp>

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

    sub.setsockopt(ZMQ_SUBSCRIBE, "color");

    zmq::message_t buf;
    while (!this->getParent()->getQuitFlag())
    {
        auto res = sub.recv(buf, zmq::recv_flags::none);
        res = sub.recv(buf, zmq::recv_flags::none);

        std::vector color_vec(buf.data<uint8_t>(), buf.data<uint8_t>() + buf.size() / sizeof(uint8_t));

        uint32_t i = 0;
        for (uint32_t y = 0; y < this->getParent()->getLedHeight(); y++)
        {
            for (uint32_t x = 0; x < this->getParent()->getLedWidth(); x++)
            {
                try
                {
                    this->getParent()->color_mat_[y * this->getParent()->getLedWidth() + x]
                        = Color(
                            color_vec[i++],
                            color_vec[i++],
                            color_vec[i++]
                        );
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }

    sub.close();
}
