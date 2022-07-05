#include <thread>

#include "Simulator.hpp"
#include "Common.hpp"
#include "Renderer.hpp"
#include "Socket.hpp"

Simulator::Simulator()
    :quit_flag_(false)
{
    // Initialize Matrix LED info
    this->setLedWidth(64);
    this->setLedHeight(32);

    this->color_mat_.resize(this->getLedWidth() * this->getLedHeight());

    for (uint32_t i = 0; i < this->getLedWidth() * this->getLedHeight(); i++)
            this->color_mat_[i] = Color(0, 0, 0);

    // Initialize components
    this->renderer_ = new Renderer(this);
    this->socket_ = new Socket(this);

    printLog("Init Simulator", true);
}

Simulator::~Simulator()
{
    delete this->renderer_;
    this->renderer_ = nullptr;

    delete this->socket_;
    this->socket_ = nullptr;

    printLog("Destroy Simulator", true);
}

void Simulator::run()
{
    uint64_t frame_num = 0;  // Frame counter

    this->runRecvSocket();

    while (!this->update())
    {
        auto quitFunc = [this]() {
            this->setQuitFlag(true);
        };

        sigaction(SIGINT, &quitFunc, NULL);
    }
}

bool Simulator::update()
{
    this->renderer_->update();

    return this->quit_flag_;
}

void Simulator::runRecvSocket()
{
    // Start receiving data in another thread
    auto recv_data = [this]()
    {
        this->socket_->run();
    };

    std::thread th_recv_data(recv_data);
    th_recv_data.detach();
}

void Simulator::updateColorData()
{

}
