#include "Simulator.hpp"

int main()
{
    Simulator *simulator = new Simulator();

    simulator->run();
    
    delete simulator;
}

