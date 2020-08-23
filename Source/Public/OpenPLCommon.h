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
    PL_ERR_MEMORY,
    /** An argument passed into the function is invalid. For example, an indices array not being a multiple of 3*/
    PL_ERR_INVALID_PARAM
};

struct JUCE_API PLQuaternion
{
    float X;
    float Y;
    float Z;
    float W;
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

/**
 * Defines one voxel cell within the voxel geometry
 */
struct JUCE_API PLVoxel
{
    float Absorptivity;
};
