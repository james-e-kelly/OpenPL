/*
  ==============================================================================

    SimulatorFDTD.h
    Created: 3 Mar 2021 11:51:55am
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "Simulator.h"

class SimulatorFDTD : public Simulator
{
    virtual void Simulate() override;
    
    ~SimulatorFDTD() { }
};
