/*
  ==============================================================================

    OpenPLCommonPrivate.cpp
    Created: 3 Feb 2021 2:27:07pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "../Public/OpenPLCommon.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

typedef Eigen::MatrixXd     VertexMatrix;
typedef Eigen::MatrixXi     IndiceMatrix;

void SetDebugCallback(PL_Debug_Callback Callback);

/**
 * Log a message to the external debug method.
 */
void Debug(const char* Message, PL_DEBUG_LEVEL Level);

/**
 * Log a message to the external debug method.
 */
void DebugLog(const char* Message);

/**
 * Log a warning message to the external debug method.
 */
void DebugWarn(const char* Message);

/**
 * Log an error message to the external debug method.
 */
void DebugError(const char* Message);

/**
 * Converts a 3D array index to a 1D index.
 */
int ThreeDimToOneDim(int X, int Y, int Z, int XSize, int YSize);

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
