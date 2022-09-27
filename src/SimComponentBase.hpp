#pragma once

class SimComponentBase
{
public:
    SimComponentBase(class Simulator* simulator)
    {
        this->simulator_ = simulator;
    }

public:
    /**
     * @brief  Get parent simulator class reference
     */
    class Simulator* getParent() const { return this->simulator_; }

private:
    /// Parent class reference
    class Simulator* simulator_;
};
