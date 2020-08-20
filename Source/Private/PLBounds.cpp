/*
  ==============================================================================

    PLBounds.cpp
    Created: 20 Aug 2020 8:16:27pm
    Author:  James Kelly

  ==============================================================================
*/

#include "PLBounds.h"

PLBounds PLBounds::CreateAABB(const PLVector& Center, const PLVector& Size)
{
    return PLBounds(Center - (Size / 2), Center + (Size / 2));
}

bool PLBounds::IsInside(const PLVector& Point) const
{
    return Point > Min && Point < Max;
}

bool PLBounds::IsInsideOrOn(const PLVector& Point) const
{
    return Point >= Min && Point <= Max;
}

bool PLBounds::Overlaps(const PLBounds& Other) const
{
    if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
    {
        return false;
    }
    
    if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
    {
        return false;
    }
    
    if ((Min.Z > Other.Max.Z) || (Other.Min.Z > Max.Z))
    {
        return false;
    }
    
    return true;
}
