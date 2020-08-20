/*
  ==============================================================================

    OpenPLTypes.h
    Created: 19 Aug 2020 12:51:40pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * Defines possible results from OpenPL API functions
 */
enum JUCE_API PL_RESULT
{
    /** Method executed successfully*/
    PL_OK,
    /** Generic error has occured without any further reason why*/
    PL_ERR,
    /** An error occured with memory. Maybe a null reference was found or memory failed to allocate/deallocate*/
    PL_ERR_MEMORY
};

/**
 * Simple Vector3D struct
 */
struct JUCE_API PLVector
{
    PLVector() = default;
    PLVector(const PLVector& Other) = default;
    PLVector(PLVector&& Other) = default;
    
    PLVector(float Scalar)
    :   X(Scalar),
        Y(Scalar),
        Z(Scalar)
    { }
    
    PLVector(float X, float Y, float Z)
    :   X(X),
        Y(Y),
        Z(Z)
    { }
    
    PLVector& operator = (const PLVector& Other)
    {
        this->X = Other.X;
        this->Y = Other.Y;
        this->Y = Other.Z;
        
        return *this;
    }
    
    //
    
    bool operator < (const PLVector& Other) const
    {
        return X < Other.X && Y < Other.Y && Z < Other.Z;
    }
    
    bool operator <= (const PLVector& Other) const
    {
        return X <= Other.X && Y <= Other.Y && Z <= Other.Z;
    }
    
    bool operator > (const PLVector& Other) const
    {
        return !(Other < *this);
    }
    
    bool operator >= (const PLVector& Other) const
    {
        return !(Other <= *this);
    }
    
    bool operator == (const PLVector& Other) const
    {
        return X == Other.X && X == Other.Y && Z == Other.Z;
    }
    
    bool operator != (const PLVector& Other) const
    {
        return !(*this == Other);
    }
    
    //
    
    PLVector operator + (const PLVector& Other) const
    {
        return PLVector(X + Other.X, Y + Other.Y, Z + Other.Y);
    }
    
    PLVector operator - (const PLVector& Other) const
    {
        return PLVector(X - Other.X, Y - Other.Y, Z - Other.Y);
    }
    
    PLVector operator + (float Scalar) const
    {
        return PLVector(X + Scalar, Y + Scalar, Z + Scalar);
    }
    
    PLVector operator - (float Scalar) const
    {
        return PLVector(X - Scalar, Y - Scalar, Z - Scalar);
    }
    
    PLVector operator += (const PLVector& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }
    
    PLVector operator -= (const PLVector& Other)
    {
        X -= Other.X;
        Y -= Other.Y;
        Z -= Other.Z;
        return *this;
    }
    
    PLVector operator += (float Scalar)
    {
        X += Scalar;
        Y += Scalar;
        Z += Scalar;
        return *this;
    }
    
    PLVector operator -= (float Scalar)
    {
        X -= Scalar;
        Y -= Scalar;
        Z -= Scalar;
        return *this;
    }
    
    PLVector operator *(float Scalar) const
    {
        return PLVector(X * Scalar, Y * Scalar, Z * Scalar);
    }
    
    PLVector operator /(float Scalar) const
    {
        const float RScale = 1.0f/Scalar;
        return *this * RScale;
    }
    
    PLVector operator *=(float Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        Z *= Scalar;
        return *this;
    }
    
    PLVector operator /=(float Scalar)
    {
        return *this *= 1.0f / Scalar;
    }
    
    //
    
    float X;
    float Y;
    float Z;
};

struct JUCE_API PLBounds
{
    PLVector Min;
    PLVector Max;
    
    PLBounds() = default;
    PLBounds(const PLBounds& Other) = default;
    PLBounds(PLBounds&& Other) = default;
    PLBounds& operator = (const PLBounds& Other)
    {
        this->Min = Other.Min;
        this->Max = Other.Max;
        
        return *this;
    }
    
    PLBounds(PLVector Min, PLVector Max)
    :   Min(Min),
        Max(Max)
    { }
    
    static PLBounds CreateAABB(const PLVector& Center, const PLVector& Size)
    {
        return PLBounds(Center - (Size / 2), Center + (Size / 2));
    }
    
    bool IsInside(const PLVector& Point) const
    {
        return Point > Min && Point < Max;
    }
    
    bool IsInsideOrOn(const PLVector& Point) const
    {
        return Point >= Min && Point <= Max;
    }
};

/**
 * Defines one voxel cell within the voxel geometry
 */
struct JUCE_API PLVoxel
{
    float Absorptivity;
};
