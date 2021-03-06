/*
  ==============================================================================

    SimulatorBasic.cpp
    Created: 4 Mar 2021 10:08:46am
    Author:  James Kelly

  ==============================================================================
*/

#include "../../Public/Simulators/SimulatorBasic.h"

void SimulatorBasic::Simulate()
{
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
    
    double Impedance = 377.f;
    
    // Time-stepped FDTD
    for (int CurrentTimeStep = 0; CurrentTimeStep < TimeSteps; CurrentTimeStep++)
    {
        // Update magnetic field for last x values
        for (int x = XSize - 1; x < XSize; ++x)
        {
            for (int y = 0; y < YSize; ++y)
            {
                for (int z = 0; z < ZSize; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    PLVoxel& PreviousVoxel = (*Lattice)[ThreeDimToOneDim(x - 1, y, z, XSize, YSize)];
                    
                    CurrentVoxel.AirPressure = PreviousVoxel.AirPressure;
                }
            }
        }
        
        // Update magnetic field
        for (int x = 0; x < XSize - 1; ++x)
        {
            for (int y = 0; y < YSize; ++y)
            {
                for (int z = 0; z < ZSize; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    const PLVoxel& NextVoxel = (*Lattice)[ThreeDimToOneDim(x + 1, y, z, XSize, YSize)];
                    
                    CurrentVoxel.AirPressure = CurrentVoxel.AirPressure + (NextVoxel.ParticleVelocityX - CurrentVoxel.ParticleVelocityX) / Impedance;
                }
            }
        }
        
        // Update electric field for first x values
        for (int x = 0; x < 1; ++x)
        {
            for (int y = 0; y < YSize; ++y)
            {
                for (int z = 0; z < ZSize; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    PLVoxel& NextVoxel = (*Lattice)[ThreeDimToOneDim(x + 1, y, z, XSize, YSize)];
                    
                    CurrentVoxel.AirPressure = NextVoxel.AirPressure;
                }
            }
        }
        
        // Update electric field
        for (int x = 1; x < XSize; ++x)
        {
            for (int y = 0; y < YSize; ++y)
            {
                for (int z = 0; z < ZSize; ++z)
                {
                    PLVoxel& CurrentVoxel = (*Lattice)[ThreeDimToOneDim(x, y, z, XSize, YSize)];
                    const PLVoxel& PreviousVoxel = (*Lattice)[ThreeDimToOneDim(x - 1, y, z, XSize, YSize)];
                    
                    CurrentVoxel.ParticleVelocityX = CurrentVoxel.ParticleVelocityX + (CurrentVoxel.AirPressure - PreviousVoxel.AirPressure) * Impedance;
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
