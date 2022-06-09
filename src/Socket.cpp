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
        auto res = sock.recv(buf, zmq::recv_flags::none);
        std::vector rgb_value(buf.data<uint8_t>(), buf.data<uint8_t>() + buf.size() / sizeof(uint8_t));  // ピクセル毎のRGB値を動的配列に格納する

        // 受信したRGB値をそれぞれ出力する
        for (auto color : rgb_value)
            std::cout << color << std::endl;

        std::cout << std::endl;
    }
}
