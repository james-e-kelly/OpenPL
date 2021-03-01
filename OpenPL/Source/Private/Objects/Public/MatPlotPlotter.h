/*
  ==============================================================================

    MatPlotPlotter.h
    Created: 1 Mar 2021 3:29:23pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "../../OpenPLCommonPrivate.h"

class MatPlotPlotter
{
public:
    
    /** Create a  MatPlotPlotter from a grid of voxels in time*/
    MatPlotPlotter(std::vector<std::vector<PLVoxel>>& SimulationGrid, int XSize, int YSize, int ZSize, int TimeSteps);
    
    void TestFunction();
    
private:
    
    std::vector<std::vector<PLVoxel>>* AllVoxels;
    
    int XSize;
    int YSize;
    int ZSize;
    int TimeSteps;
};
