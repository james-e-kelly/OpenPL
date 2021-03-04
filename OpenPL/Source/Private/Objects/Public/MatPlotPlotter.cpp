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
#include <cstdlib>

using namespace matplot;

MatPlotPlotter::MatPlotPlotter(const std::vector<std::vector<PLVoxel>>& SimulationGrid, int XSize, int YSize, int ZSize, int TimeSteps)
: Lattice(SimulationGrid),
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

void MatPlotPlotter::PlotOneDimension(int YIndex, int ZIndex)
{
    figure_handle PlotFigure = figure(false);
    
    for (int TimeStep = 0; TimeStep < TimeSteps; TimeStep++)
    {
        PlotFigure->current_axes()->clear();
        
        std::vector<double> XPoints;
        std::vector<double> YPoints;
        
        for (int X = 0; X < XSize; X++)
        {
            int Index = ThreeDimToOneDim(X, YIndex, ZIndex, XSize, YSize);
            
            const PLVoxel& Voxel = Lattice[Index][TimeStep];
            
            XPoints.push_back(static_cast<double>(X));
            YPoints.push_back(Voxel.AirPressure);
        }
        
        PlotFigure->current_axes()->plot(XPoints, YPoints);
        
        PlotFigure->current_axes()->ylim({-1,1});
        
        PlotFigure->current_axes()->draw();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
