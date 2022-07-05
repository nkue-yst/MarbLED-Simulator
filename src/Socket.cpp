#include <vector>

#include "Socket.hpp"
#include "Common.hpp"
#include "Simulator.hpp"

Socket::Socket(Simulator* simulator)
    :SimComponentBase(simulator)
{
    printLog("Create socket component", true);
}

Socket::~Socket()
{
    printLog("Destroy Socket", true);
}

void Socket::run()
{
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.bind("tcp://127.0.0.1:44100");

    printLog("Init Socket", true);

    zmq::message_t buf;

    while (!this->getParent()->getQuitFlag())
    {
        this->getParent()->mutex_color_mat_.lock();

        auto res = sock.recv(buf, zmq::recv_flags::none);
        std::vector rgb_value(buf.data<uint16_t>(), buf.data<uint16_t>() + buf.size() / sizeof(uint16_t));  // ピクセル毎のRGB値を動的配列に格納する

        Color color(rgb_value.at(1), rgb_value.at(2), rgb_value.at(3));
        this->getParent()->color_mat_.at(rgb_value.at(0)) = color;

        this->getParent()->mutex_color_mat_.unlock();
    }
}
