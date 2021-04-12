/*
  ==============================================================================

    Simulator.h
    Created: 3 Mar 2021 11:51:46am
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include <vector>
#include "OpenPLCommonPrivate.h"

class PL_SCENE;

class Simulator
{
public:
    
    void Init(PL_SCENE* Scene, PL_VOXEL_GRID& Voxels, PL_SIMULATION_SETTINGS& Settings);
    
    virtual void Simulate() { }
    
    const std::vector<std::vector<PLVoxel>>& GetSimulatedLattice() const;
    
    const PL_SCENE* GetScene() const;
    
    float GetSamplingRate() const;
    
    virtual ~Simulator() { }
    
protected:
    
    void GaussianPulse();
    
protected:
    
    int XSize;
    int YSize;
    int ZSize;
    int SquareSize;
    int CubeSize;
    int TimeSteps;
    double SamplingRate;
    double UpdateCoefficents;
    PL_SIMULATION_SETTINGS Settings;
    
    /**The 3D cube of voxels. Attached to the scene/geometry*/
    std::vector<PLVoxel>* Lattice;
    
    /**The 3D cube of voxels with values at each time step*/
    std::vector<std::vector<PLVoxel>> SimulatedLattice;
    
    /**Gaussian pulse*/
    std::vector<double> Pulse;
    
    PL_SCENE* OwningScene;
};
