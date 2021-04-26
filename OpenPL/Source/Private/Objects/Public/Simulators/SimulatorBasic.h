/*
  ==============================================================================

    SimulatorBasic.h
    Created: 4 Mar 2021 10:08:37am
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "Simulator.h"

class SimulatorBasic : public Simulator
{
    virtual void Simulate(int SimulateVoxelIndex) override;
    
    ~SimulatorBasic() { }
};
