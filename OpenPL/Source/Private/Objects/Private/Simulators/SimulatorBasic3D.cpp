/*
  ==============================================================================

    SimulatorBasic3D.cpp
    Created: 15 Mar 2021 10:08:31am
    Author:  James Kelly

  ==============================================================================
*/

#include "Simulators/SimulatorBasic3D.h"

void SimulatorBasic3D::Simulate(int SimulateVoxelIndex)
{
    // Courant number contains the ratio of the temporal step to the spatial step
    // So if we are simulating 1 sec per 1 meter, the courant is 1 (or Unity)
    
    // Reset all pressure and velocity
    {
        for(auto& Voxel : *Lattice)
        {
            Voxel.AirPressure = 0.0f;
            Voxel.ParticleVelocityX = 0.0f;
            Voxel.ParticleVelocityY = 0.0f;
            Voxel.ParticleVelocityZ = 0.0f;
        }
    }
    
    const double Impedance = 377.f;
    
    // Time-stepped FDTD
    for (int CurrentTimeStep = 0; CurrentTimeStep < TimeSteps; CurrentTimeStep++)
    {
        // Update electric field
        for (int x = 0; x < XSize - 1; ++x)
        {
            for (int y = 0; y < YSize - 1; ++y)
            {
                for (int z = 0; z < ZSize - 1; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    const PLVoxel& NextXVoxel = (*Lattice)[ThreeDimToOneDim(x + 1, y, z, XSize, YSize)];
                    const PLVoxel& NextYVoxel = (*Lattice)[ThreeDimToOneDim(x, y + 1, z, XSize, YSize)];
                    const PLVoxel& NextZVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z + 1, XSize, YSize)];
                }
            }
        }
        
        // Update magnetic field
        for (int x = 1; x < XSize; ++x)
        {
            for (int y = 1; y < YSize; ++y)
            {
                for (int z = 1; z < ZSize; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    const PLVoxel& PreviousXVoxel = (*Lattice)[ThreeDimToOneDim(x - 1, y, z, XSize, YSize)];
                    const PLVoxel& PreviousYVoxel = (*Lattice)[ThreeDimToOneDim(x, y - 1, z, XSize, YSize)];
                    const PLVoxel& PreviousZVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z - 1, XSize, YSize)];
                }
            }
        }
        
        // Add response
        {
            for (int i = 0; i < CubeSize; i++)
            {
                SimulatedLattice[i][CurrentTimeStep] = (*Lattice)[i];
            }
        }
        
        // Add pulse
        (*Lattice)[4].AirPressure += Pulse[CurrentTimeStep];
    }
}

