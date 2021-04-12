/*
  ==============================================================================

    Analyser.h
    Created: 12 Apr 2021 8:00:55pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommon.h"

class Simulator;

/**
 * Takes the simulation result at a location and builds an impulse response. In future versions, will encode data as simple parameters
 */
class Analyser
{
public:
    
    void Encode(Simulator* Simulator, PLVector EncodingPosition);
};
