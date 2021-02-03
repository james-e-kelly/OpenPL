/*
  ==============================================================================

    OpenPLCommonPrivate.cpp
    Created: 3 Feb 2021 2:27:07pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

typedef Eigen::MatrixXd     VertexMatrix;
typedef Eigen::MatrixXi     IndiceMatrix;

static PL_Debug_Callback DebugCallback = nullptr;

/**
 * Log a message to the external debug method.
 */
static void Debug(const char* Message, PL_DEBUG_LEVEL Level)
{
    if (DebugCallback)
    {
        DebugCallback(Message, Level);
    }
}

/**
 * Log a message to the external debug method.
 */
static void DebugLog(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_LOG);
}

/**
 * Log a warning message to the external debug method.
 */
static void DebugWarn(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_WARN);
}

/**
 * Log an error message to the external debug method.
 */
static void DebugError(const char* Message)
{
    Debug(Message, PL_DEBUG_LEVEL_ERR);
}

/**
 * Converts a 3D array index to a 1D index.
 */
static int ThreeDimToOneDim(int X, int Y, int Z, int XSize, int YSize)
{
    // The X,Y and Z sides of the lattice are all combined into the rows of the matrix
    // To access the correct index, we have to convert 3D index to a 1D index
    // https://stackoverflow.com/questions/16790584/converting-index-of-one-dimensional-array-into-two-dimensional-array-i-e-row-a#16790720
    // The actual vertex positions of the index are contained within 3 columns
    return  X + Y * XSize + Z * XSize * YSize;
}

/**
 * Defines one voxel cell within the voxel geometry
 */
struct JUCE_API PLVoxel
{
    Eigen::Vector3d WorldPosition;
    float Absorptivity;
};

/**
 * Defines a simple mesh with vertices and indices.
 */
struct PL_MESH
{
    VertexMatrix Vertices;
    IndiceMatrix Indices;
};

/**
 * Defines the voxel lattice of a geometric scene.
 */
struct PL_VOXEL_GRID
{
    /** Bounding box the voxels are within. Can be used to quickly check if a point is within the lattice*/
    Eigen::AlignedBox<double, 3> Bounds;
    /** Contains size of each dimension of the lattice. Ie Size(0,0) would return the size of the lattice along the X axis*/
    Eigen::Matrix<int,1,3,1,1,3> Size;
    /** 1D vector containing all voxels. Contains all voxels witin the lattice so must convert 3D indexes to 1D before accessing*/
    std::vector<PLVoxel> Voxels;
    /** Width aka Height aka Depth of each voxel. With CenterPositions and Voxels, can use this to create bounding boxes of each voxel*/
    float VoxelSize;
};
