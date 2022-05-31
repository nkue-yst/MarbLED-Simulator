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

    this->color_mat_.resize(this->getLedHeight());
    for (auto& color_vec : this->color_mat_)
        color_vec.resize(this->getLedWidth());

    for (uint32_t y = 0; y < this->getLedHeight(); y++)
        for (uint32_t x = 0; x < this->getLedWidth(); x++)
            this->color_mat_[y][x] = Color(0, 0, 0);

    //this->color_mat[5][2] = Color(255, 255, 255);  // Draw point for debug

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
        std::cout << frame_num << std::endl;  // Debug print for frame count

        // Test for quit by time
        if (frame_num >= 10)
            this->quit_flag_ = true;

        frame_num++;
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
