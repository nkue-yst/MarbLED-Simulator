#include "Socket.hpp"
#include "Common.hpp"
#include "Simulator.hpp"

Socket::Socket(Simulator* simulator)
    :SimComponentBase(simulator)
{
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.bind("tcp://127.0.0.1:44100");

    printLog("Init Socket", true);
}

Socket::~Socket()
{
    printLog("Destroy Socket", true);
}

void Socket::run()
{
    while (!this->getParent()->getQuitFlag())
    {
        std::cout << "--- Socket is running ---" << std::endl;
        delay(50);
    }
}
