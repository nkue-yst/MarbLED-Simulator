#include "Socket.hpp"

#include <cstring>
#include <unistd.h>
#include <vector>

#include <sys/socket.h>
#include <sys/un.h>

#include "Common.hpp"
#include "Simulator.hpp"

#define UNIX_SOCKET_PATH "/tmp/tll/unix_socket"

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
    struct sockaddr_un dest_addr;
    std::memset(&dest_addr, 0, sizeof(struct sockaddr_un));

    dest_addr.sun_family = AF_UNIX;
    std::strcpy(dest_addr.sun_path, UNIX_SOCKET_PATH);
    std::remove(dest_addr.sun_path);

    int32_t sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printLog("Create socket", false);
        close(sock);
        delete this->getParent();
        exit(1);
    }

    if (bind(sock, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_un)) == -1)
    {
        printLog("Bind socket", false);
        close(sock);
        delete this->getParent();
        exit(1);
    }

    if (listen(sock, 1) == -1)
    {
        printLog("Listen connection", false);
        close(sock);
        delete this->getParent();
        exit(1);
    }

    int32_t dest_sock;    // 接続先ソケット
    
    std::cout << "Waiting connect..." << std::endl;
    
    dest_sock = accept(sock, NULL, NULL);
    if (dest_sock == -1)
    {
        printLog("Accept socket connection", false);
        close(sock);
        delete this->getParent();
        exit(1);
    }
    printLog("Connected!", true);

    while (!this->getParent()->getQuitFlag())
    {
        std::vector<uint8_t> color_vec;
        color_vec.resize(this->getParent()->getLedWidth() * this->getParent()->getLedHeight() * 3);

        recv(dest_sock, color_vec.data(), this->getParent()->getLedWidth() * this->getParent()->getLedHeight() * 3, 0);

        uint32_t i = 0;
        for (uint32_t y = 0; y < this->getParent()->getLedHeight(); y++)
        {
            for (uint32_t x = 0; x < this->getParent()->getLedWidth(); x++)
            {
                try
                {
                    this->getParent()->color_mat_.at(y * this->getParent()->getLedWidth() + x)
                        = Color(
                            color_vec.at(i++),
                            color_vec.at(i++),
                            color_vec.at(i++)
                        );
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }

    close(dest_sock);
    close(sock);
    std::remove(UNIX_SOCKET_PATH);
}
