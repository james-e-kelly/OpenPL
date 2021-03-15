/*
  ==============================================================================

    MatPlotPlotter.h
    Created: 1 Mar 2021 3:29:23pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommonPrivate.h"

class MatPlotPlotter
{
public:
    
    /** Create a  MatPlotPlotter from a grid of voxels in time*/
    MatPlotPlotter(const std::vector<std::vector<PLVoxel>>& SimulationGrid, int XSize, int YSize, int ZSize, int TimeSteps);
    
    void TestFunction();
    
    /**Plot all points of the X axis in time with a constant Y and Z value*/
    void PlotOneDimension(int YIndex = 0, int ZIndex = 0);
    
    void PlotOneDimensionWaterfall(int YIndex = 0, int ZIndex = 0);
    
private:
    
    const std::vector<std::vector<PLVoxel>>& Lattice;
    
    int XSize;
    int YSize;
    int ZSize;
    int TimeSteps;
};
