/*
  ==============================================================================

    OpenPLCommonPrivate.cpp
    Created: 3 Feb 2021 2:52:29pm
    Author:  James Kelly

  ==============================================================================
*/

#include "OpenPLCommonPrivate.h"

PL_Debug_Callback DebugCallback = nullptr;

void SetDebugCallback(PL_Debug_Callback Callback)
{
    DebugCallback = Callback;
}

/**
 * Log a message to the external debug method.
 */
void Debug(const char* Message, PL_DEBUG_LEVEL Level)
{
    if (DebugCallback)
    {
        DebugCallback(Message, Level);
    }
}

/**
 * Log a message to the external debug method.
 */
void DebugLog(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_LOG);
}

/**
 * Log a warning message to the external debug method.
 */
void DebugWarn(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_WARN);
}

/**
 * Log an error message to the external debug method.
 */
void DebugError(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_ERR);
}

/**
 * Converts a 3D array index to a 1D index.
 */
int ThreeDimToOneDim(int X, int Y, int Z, int XSize, int YSize)
{
    // The X,Y and Z sides of the lattice are all combined into the rows of the matrix
    // To access the correct index, we have to convert 3D index to a 1D index
    // https://stackoverflow.com/questions/16790584/converting-index-of-one-dimensional-array-into-two-dimensional-array-i-e-row-a#16790720
    // The actual vertex positions of the index are contained within 3 columns
    return  X + Y * XSize + Z * XSize * YSize;
}

void IndexToThreeDim(int Index, int XSize, int YSize, int& OutX, int& OutY, int& OutZ)
{
    OutZ = Index / (XSize * YSize);
    Index = Index % (XSize * YSize);
    OutY = Index / XSize;
    OutX = Index % XSize;
}
