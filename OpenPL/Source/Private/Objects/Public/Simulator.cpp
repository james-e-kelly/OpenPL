/*
  ==============================================================================

    Simulator.cpp
    Created: 3 Mar 2021 12:01:36pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/Simulator.h"

void Simulator::Init(PL_VOXEL_GRID& Voxels, PL_SIMULATION_SETTINGS& Settings)
{
    this->XSize = Voxels.Size(0,0);
    this->YSize = Voxels.Size(0,1);
    this->ZSize = Voxels.Size(0,2);
    this->SquareSize = XSize * YSize;
    this->CubeSize = XSize * YSize * ZSize;
    this->TimeSteps = Settings.TimeSteps;
    this->Lattice = &Voxels.Voxels;
    this->Settings = Settings;
    
    SimulatedLattice.resize(CubeSize);
    
    for (std::vector<PLVoxel>& VoxelVector : SimulatedLattice)
    {
        VoxelVector.resize(TimeSteps);
    }
    
    const double SpeedOfSound = 343.21f;
    const double MinWaveLength = SpeedOfSound / Settings.Resolution;    // divided by min frequency for simulation. 275 is pretty low and should be fast
    const double MetersPerGridCell = MinWaveLength / 3.5f;
    const double SecondsPerSample = MetersPerGridCell / (SpeedOfSound * 1.5f);
    SamplingRate  = 1.0f / SecondsPerSample;
    
    // Need to calculate these coefficents properly
    UpdateCoefficents = SpeedOfSound * SecondsPerSample / MetersPerGridCell;
    
    GaussianPulse();
}

const std::vector<std::vector<PLVoxel>>& Simulator::GetSimulatedLattice() const
{
    return SimulatedLattice;
}

void Simulator::GaussianPulse()
{
    Pulse.resize(TimeSteps);
    
    const float maxFreq = static_cast<float>(Settings.Resolution);
    const float pi = std::acos(-1);
    float sigma = 1.0f / (0.5 * pi * maxFreq);

    const float delay = 2*sigma;
    const float dt = 1.0f / SamplingRate;

    for (unsigned i = 0; i < TimeSteps; ++i)
    {
        double t = static_cast<double>(i) * dt;
        double val = std::exp(-(t - delay) * (t - delay) / (sigma * sigma));
        Pulse[i] = val;
    }
}
