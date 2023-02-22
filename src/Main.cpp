#include <cstring>

#include "Simulator.hpp"

int main(int argc, char** argv)
{
    Simulator *simulator = new Simulator();

    if (argc == 3)
    {
        if (std::strcmp(argv[1], "-d") == 0)
        {
            std::string dest_ip(argv[2]);
            simulator->run(dest_ip);
        }
    }
    else if (argc == 1)
    {
        simulator->run();
    }
    
    delete simulator;
}

