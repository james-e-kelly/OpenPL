/*
  ==============================================================================

    FreeGrid.h
    Created: 8 May 2021 1:06:24pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommonPrivate.h"

class PL_SCENE;

/**
 * Handles caputuring the impulse response of an empty grid with no geometry. The information taken from the free grid is used to calucalte occlusion values
 */
class FreeGrid
{
public:
    
    void Init(PL_SCENE* Scene);
    
    double GetFreeEnergy(int ListenerIndexX, int ListenerIndexY, int EmitterIndexX, int EmitterIndexY);
    double GetEnergyAtOneMeter() const;
    
private:
    double SimulateFreeFieldEnergy(PL_SCENE* Scene);
    double CalculateEFree(const std::vector<PLVoxel>& Response, float SamplingRate) const;

    float VoxelSize;
    double FreeEnergy;
};
