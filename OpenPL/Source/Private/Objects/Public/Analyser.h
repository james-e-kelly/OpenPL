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
    
    /**
     * Writes an impulse response file to disk.
     *
     * @param Simulator Simulator to take data from
     * @param EncodingPosition Position to make the impulse response from
     * @param OutVoxelIndex
     
     */
    void Encode(Simulator* Simulator, PLVector EncodingPosition, int* OutVoxelIndex);
    
    void GetOcclusion(Simulator* Simulator, PLVector EncodingPosition, float* OutOcclusion);
};
