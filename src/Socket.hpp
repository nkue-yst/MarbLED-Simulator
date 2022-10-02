#pragma once

#include "SimComponentBase.hpp"

class Socket : public SimComponentBase
{
public:
    Socket(class Simulator* simulator);
    ~Socket();

    void run();
};
