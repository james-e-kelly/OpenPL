/*
  ==============================================================================

    FreeGrid.cpp
    Created: 8 May 2021 1:06:34pm
    Author:  James Kelly

  ==============================================================================
*/

#include "FreeGrid.h"
#include "PL_SCENE.h"
#include "Simulators/Simulator.h"

void FreeGrid::Init(PL_SCENE* Scene)
{
    Scene->GetSceneVoxelSize(&VoxelSize);
    FreeEnergy = SimulateFreeFieldEnergy(Scene);
}

double FreeGrid::GetFreeEnergy(int ListenerIndexX, int ListenerIndexY, int EmitterIndexX, int EmitterIndexY)
{
    // find Euclidean distance between listener and emitter
    double lX = (double)ListenerIndexX * VoxelSize;
    double lY = (double)ListenerIndexY * VoxelSize;
    double eX = (double)EmitterIndexX * VoxelSize;
    double eY = (double)EmitterIndexY * VoxelSize;

    double r = std::sqrt((eX - lX) * (eX - lX) +
        (eY - lY) * (eY - lY));
    if (r == 0.f)
    {
        return FreeEnergy;
    }

    // 2D propagation: energy decays as 1/r
    return FreeEnergy / r;
}

double FreeGrid::GetEnergyAtOneMeter() const
{
    return FreeEnergy;
}

double FreeGrid::SimulateFreeFieldEnergy(PL_SCENE* Scene)
{
    PLVector ScenePosition;
    Scene->GetScenePosition(&ScenePosition);

    Scene->Simulate(ScenePosition);
    // generate a set of IRs in the grid, calculate the free energy
    Simulator* Simulator;
    Scene->GetSimulator(&Simulator);
    
    PLVector OneMeterInFrontOfListerner = ScenePosition + PLVector(1,0,0);
    
    int EmitterIndex;
    Scene->GetVoxelIndexOfPosition(OneMeterInFrontOfListerner, &EmitterIndex);
    
    const std::vector<std::vector<PLVoxel>>& Lattice = Simulator->GetSimulatedLattice();
    
    const std::vector<PLVoxel>& Response = Lattice[EmitterIndex];
    
    float SamplingRate = Simulator->GetSamplingRate();
    double freeFieldEnergy = CalculateEFree(Response, SamplingRate);

    // discrete distance on grid
    const double r = double(OneMeterInFrontOfListerner.X - ScenePosition.X) * VoxelSize;
    // Normalize to exactly 1m assuming 1/r energy attenuation
    freeFieldEnergy *= r;

    return freeFieldEnergy;
}

double FreeGrid::CalculateEFree(const std::vector<PLVoxel>& Response, float SamplingRate) const
{
    // Dry duration, plus delay to get 1m away
    int NumSamples = (int)((0.01f) * ((float)SamplingRate)) + (int)(((float)1.f / 343.21f) * SamplingRate);
    if (NumSamples > Response.size())
    {
        DebugError("Samples longer than time steps");
        return 0;
    }
    double efree = 0.f;

    // sum up square of signal values
    for (int i = 0; i < NumSamples; ++i)
    {
        efree += Response[i].AirPressure * Response[i].AirPressure;
    }

    return efree;
}
