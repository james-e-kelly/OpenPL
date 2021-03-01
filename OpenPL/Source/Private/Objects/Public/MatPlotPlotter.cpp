/*
  ==============================================================================

    MatPlotPlotter.cpp
    Created: 1 Mar 2021 3:29:23pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/MatPlotPlotter.h"
#include <matplot/matplot.h>
#include <thread>
#include <chrono>

using namespace matplot;

MatPlotPlotter::MatPlotPlotter(std::vector<std::vector<PLVoxel>>& SimulationGrid, int XSize, int YSize, int ZSize, int TimeSteps)
: AllVoxels(&SimulationGrid),
XSize(XSize),
YSize(YSize),
ZSize(ZSize),
TimeSteps(TimeSteps)
{
    
}

void MatPlotPlotter::TestFunction()
{
    figure_handle f = figure();
    
    
    
    std::vector<double> y = {75,  91,  105, 123.5, 131,  150,
                                 179, 203, 226, 249,   281.5};
    
    f->current_axes()->clear();
    f->current_axes()->bar(y);
    f->draw();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    std::vector<double> z = {109,  91,  105, 20.5, 131,  50,
                                 179, 203, 25, 2649,   81.5};
    
    f->current_axes()->clear();
    f->current_axes()->bar(z);
    f->draw();
}
