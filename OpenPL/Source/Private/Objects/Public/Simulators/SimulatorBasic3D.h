/*
  ==============================================================================

    SimulatorBasic3D.h
    Created: 15 Mar 2021 10:08:22am
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "Simulator.h"

class SimulatorBasic3D : public Simulator
{
    virtual void Simulate(int SimulateVoxelIndex) override;
    
    ~SimulatorBasic3D() { }
};

/**
 THIS IS ALL ELECTROMAGNETICS!!! NOT ACOUSTICS!!!!
 
 1D
 ez[m] = ez[m] + (hy[m] - hy[m - 1]) * 377;
 hy[m] = hy[m] + (ez[m + 1] - ez[m]) / 377;
 
 2D
 Hx(m, n) = Chxh(m, n) * Hx(m, n) - Chxe(m, n) * (Ez(m, n + 1) - Ez(m, n));
 Hy(m, n) = Chyh(m, n) * Hy(m, n) + Chye(m, n) * (Ez(m + 1, n) - Ez(m, n));
 Ez(m, n) = Ceze(m, n) * Ez(m, n) + Cezh(m, n) * ((Hy(m, n) - Hy(m - 1, n)) - (Hx(m, n) - Hx(m, n - 1)));
 
 3D
 Hx(m, n, p) = Chxh(m, n, p) * Hx(m, n, p) + Chxe(m, n, p) * ((Ey(m, n, p + 1) - Ey(m, n, p)) - (Ez(m, n + 1, p) - Ez(m, n, p)));
 Hy(m, n, p) = Chyh(m, n, p) * Hy(m, n, p) + Chye(m, n, p) * ((Ez(m + 1, n, p) - Ez(m, n, p)) - (Ex(m, n, p + 1) - Ex(m, n, p)));
 Hz(m, n, p) = Chzh(m, n, p) * Hz(m, n, p) + Chze(m, n, p) * ((Ex(m, n + 1, p) - Ex(m, n, p)) - (Ey(m + 1, n, p) - Ey(m, n, p)));
 Ex(m, n, p) = Cexe(m, n, p) * Ex(m, n, p) + Cexh(m, n, p) * ((Hz(m, n, p) - Hz(m, n - 1, p)) - (Hy(m, n, p) - Hy(m, n, p - 1)));
 Ey(m, n, p) = Ceye(m, n, p) * Ey(m, n, p) + Ceyh(m, n, p) * ((Hx(m, n, p) - Hx(m, n, p - 1)) - (Hz(m, n, p) - Hz(m - 1, n, p)));
 Ez(m, n, p) = Ceze(m, n, p) * Ez(m, n, p) + Cezh(m, n, p) * ((Hy(m, n, p) - Hy(m - 1, n, p)) - (Hx(m, n, p) - Hx(m, n - 1, p)));
 
 1D
 
 2D
 for (mm = 0; mm < SizeX; mm++)
    for (nn = 0; nn < SizeY - 1; nn++)
        Hx(mm, nn) = Chxh(mm, nn) * Hx(mm, nn) - Chxe(mm, nn) * (Ez(mm, nn + 1) - Ez(mm, nn));
 
 for (mm = 0; mm < SizeX - 1; mm++)
    for (nn = 0; nn < SizeY; nn++)
        Hy(mm, nn) = Chyh(mm, nn) * Hy(mm, nn) + Chye(mm, nn) * (Ez(mm + 1, nn) - Ez(mm, nn));
 
 for (mm = 1; mm < SizeX - 1; mm++)
    for (nn = 1; nn < SizeY - 1; nn++)
        Ez(mm, nn) = Ceze(mm, nn) * Ez(mm, nn) + Cezh(mm, nn) * ((Hy(mm, nn) - Hy(mm - 1, nn)) - (Hx(mm, nn) - Hx(mm, nn - 1)));
 
 3D
 */

/**
 ACOUSTICS
 
 2D
 Vx(m,n) = Vx(m,n) - Cvxp(m,n) * (Pr(m+1,n) - Pr(m,n));
 Vy(m,n) = Vy(m,n) - Cvyp(m,n) * (Pr(m,n+1) - Pr(m,n));
 Pr(m,n) = Pr(m,n) - Cprv(m,n) * ((Vx(m,n) - Vx(m-1,n)) + (Vy(m,n) - Vy(m,n-1)));
 */
