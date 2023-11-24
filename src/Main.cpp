#include <cstring>
#include <iostream>

#include "Simulator.hpp"

int main(int argc, char** argv)
{
    Simulator *simulator = new Simulator();

    // Handling the arguments
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 < argc)
            {
                std::string dest_ip(argv[++i]);
                simulator->run(dest_ip);
            }
            else
            {
                std::cout << "Error: -d option requires one argument." << std::endl;
                continue;
            }
        }
        else if (std::strcmp(argv[i], "-W") == 0)
        {
            if (i + 1 < argc)
            {
                if (std::stoi(argv[i + 1]) % PANEL_WIDTH != 0)
                {
                    std::cout << "Error: -W option requires a multiple of " << PANEL_WIDTH << "." << std::endl;
                    continue;
                }
                else
                {
                    simulator->setLedWidth(std::stoi(argv[++i]));
                }
            }
            else
            {
                std::cout << "Error: -W option requires one argument." << std::endl;
                continue;
            }
        }
        else if (std::strcmp(argv[i], "-H") == 0)
        {
            if (i + 1 < argc)
            {
                if (std::stoi(argv[i + 1]) % PANEL_HEIGHT != 0)
                {
                    std::cout << "Error: -H option requires a multiple of " << PANEL_HEIGHT << "." << std::endl;
                    continue;
                }
                else
                {
                    simulator->setLedHeight(std::stoi(argv[++i]));
                }
            }
            else
            {
                std::cout << "Error: -H option requires one argument." << std::endl;
                continue;
            }
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -d <ip>     Set destination IP address" << std::endl;
            std::cout << "  -W <width>  Set Matrix LED width" << std::endl;
            std::cout << "  -H <height> Set Matrix LED height" << std::endl;
            std::cout << "  --help      Show this help" << std::endl;

            return 0;
        }
        else
        {
            std::cout << "Error: Unknown option: " << argv[i] << std::endl << std::endl;

            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -d <ip>    Set destination IP address" << std::endl;
            std::cout << "  -W <width> Set Matrix LED width" << std::endl;
            std::cout << "  -H <height> Set Matrix LED height" << std::endl;
            std::cout << "  --help     Show this help" << std::endl;
            return 1;
        }
    }

    simulator->run();

    delete simulator;
}

