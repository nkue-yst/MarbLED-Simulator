#pragma once

#include <string>

#include "SimComponentBase.hpp"

class Socket : public SimComponentBase
{
public:
    Socket(class Simulator* simulator);
    ~Socket();

    void run(std::string dest_ip);
};
