/*
  ==============================================================================

    BoundingBox.cpp
    Created: 20 Aug 2020 8:15:08pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../Public/OpenPLCommon.h"

/**
 * Represents a AABB.
 */
struct JUCE_API PLBounds
{
    PLVector Min;
    PLVector Max;
    
    PLBounds(PLVector Min, PLVector Max);
    
    static PLBounds CreateAABB(const PLVector& Center, const PLVector& Size);
    
    bool IsInside(const PLVector& Point) const;
    
    bool IsInsideOrOn(const PLVector& Point) const;
    
    bool Overlaps(const PLBounds& Other) const;
};
