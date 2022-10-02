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
        /* ピクセルIDの受信 */
        int8_t index_str[32] = {0};
        recv(dest_sock, index_str, sizeof(index_str), 0);
        uint16_t index = std::atoi((const char*)index_str);

        /* 色情報の受信 */
        uint8_t color[3] = {0};
        recv(dest_sock, color, sizeof(color), 0);

        this->getParent()->color_mat_.at(index) = Color(color[0], color[1], color[2]);
    }

    close(dest_sock);
    close(sock);
    std::remove(UNIX_SOCKET_PATH);
}
