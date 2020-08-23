/*
  ==============================================================================

    OpenPL.cpp
    Created: 15 Aug 2020 8:12:46pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/OpenPL.h"
#include "PLBounds.h"

#include <boost/format.hpp>

// Hide warnings from external libraries because it's not up to use to fix them
#pragma warning(push, 0)
#include <forward_list>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/voxel_grid.h>
#pragma warning(pop)

typedef Eigen::MatrixXd     VertexMatrix;
typedef Eigen::MatrixXi     IndiceMatrix;

/**
 * Defines simple mesh with vertices and indices.
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
    /** Matrix of all center positions of the voxels. Matrix is XSize*YSize*ZSize by 3*/
    Eigen::Matrix<double,-1,-1,0,-1,-1> CenterPositions;
    /** Contains size of each dimension of the lattice. Ie Size(0,0) would return the size of the lattice along the X axis*/
    Eigen::Matrix<int,1,3,1,1,3> Size;
    /** 1D vector containing all voxels. Contains all voxels witin the lattice so must convert 3D indexes to 1D before accessing*/
    std::vector<PLVoxel> Voxels;
    /** Width aka Height aka Depth of each voxel. With CenterPositions and Voxels, can use this to create bounding boxes of each voxel*/
    float VoxelSize;
    
};

class PL_SYSTEM
{
public:
    PL_SYSTEM()
    { }
    
    ~PL_SYSTEM()
    { }
    
    PL_RESULT AddScene(PL_SCENE* Scene)
    {
        Scenes.push_front(std::unique_ptr<PL_SCENE>(Scene));
        return PL_OK;
    }
    
    PL_RESULT RemoveScene(PL_SCENE* Scene)
    {
        Scenes.remove_if([Scene](std::unique_ptr<PL_SCENE>& Elem) { return Elem.get() == Scene; });
        return PL_OK;
    }

private:
    std::forward_list<std::unique_ptr<PL_SCENE>> Scenes;
};

class PL_SCENE
{
public:
    
    PL_SCENE(PL_SYSTEM* System)
    :   OwningSystem(System)
    { }
    
    PL_RESULT GetSystem(PL_SYSTEM** OutSystem)
    {
        *OutSystem = OwningSystem;
        return PL_OK;
    }
    
    PL_RESULT AddMesh(PL_MESH& Mesh, int& OutIndex)
    {
        Meshes.push_back(Mesh);
        OutIndex = static_cast<int>(Meshes.size()) - 1;
        return PL_OK;
    }
    
    PL_RESULT RemoveMesh(int Index)
    {
        Meshes.erase(Meshes.begin()+Index);
        return PL_OK;
    }
    
    std::vector<PL_MESH>& GetMeshes()
    {
        return Meshes;
    }
    
    PL_RESULT OpenOpenGLDebugWindow()
    {
        igl::opengl::glfw::Viewer viewer;
        for (auto& Mesh : Meshes)
        {
            viewer.data().set_mesh(Mesh.Vertices, Mesh.Indices);
            viewer.append_mesh(true);
        }
        
        int Success = viewer.launch();
        
        if (Success == EXIT_SUCCESS)
        {
            return PL_OK;
        }
        
        return PL_ERR;
    }
    
    void SetVoxels(PL_VOXEL_GRID& InVoxels)
    {
        Voxels = InVoxels;
    }
    
private:
    PL_SYSTEM* OwningSystem;
    
    std::vector<PL_MESH> Meshes;
    
    PL_VOXEL_GRID Voxels;
};

static PL_Debug_Callback DebugCallback = nullptr;

PL_RESULT PL_Debug_Initialize (PL_Debug_Callback Callback)
{
    DebugCallback = Callback;
    return PL_OK;
}

void Debug(const char* Message, PL_DEBUG_LEVEL Level)
{
    if (DebugCallback)
    {
        DebugCallback(Message, Level);
    }
}

PL_RESULT PL_System_Create (PL_SYSTEM** OutSystem)
{
    if (!OutSystem)
    {
        return PL_ERR_MEMORY;
    }
    
    *OutSystem = new PL_SYSTEM();
    return PL_OK;
}

PL_RESULT PL_System_Release (PL_SYSTEM* System)
{
    delete System;
    return PL_OK;
}

PL_RESULT PL_System_CreateScene(PL_SYSTEM* System, PL_SCENE** OutScene)
{
    if (!System)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    PL_SCENE* CreatedScene = new PL_SCENE(System);
    *OutScene = CreatedScene;
    PL_RESULT Result = System->AddScene(CreatedScene);
    return Result;
}

PL_RESULT PL_Scene_Release(PL_SCENE* Scene)
{
    if (!Scene)
    {
        return PL_OK;
    }
    
    PL_SYSTEM* System = nullptr;
    Scene->GetSystem(&System);
    if (System)
    {
        return System->RemoveScene(Scene);
    }
    return PL_ERR_MEMORY;
}

PL_RESULT PL_Scene_AddMesh(PL_SCENE* Scene, PLVector* WorldPosition, PLQuaternion* WorldRotation, PLVector* WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex)
{
    // Return if any pointers are invalid
    if (!Scene || !WorldPosition || !WorldRotation || !Vertices || !Indices)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    // Indices must be multiple of 3
    if (IndicesLength % 3 != 0)
    {
        return PL_ERR_INVALID_PARAM;
    }

    std::string PositionString;
    PositionString.append("X: ");
    PositionString.append(std::to_string(WorldPosition->X));
    PositionString.append(", Y: ");
    PositionString.append(std::to_string(WorldPosition->Y));
    PositionString.append(", Z: ");
    PositionString.append(std::to_string(WorldPosition->Z));
    
    Debug(PositionString.c_str(), PL_DEBUG_LEVEL_LOG);
    
    // Copy vertices into a matrix
    VertexMatrix EigenVertices (VerticesLength, 3); // Eigen::Matrix(Index rows, Index columns);
    
    Eigen::Vector3d Scale (WorldScale->X, WorldScale->Y, WorldScale->Z);
    Eigen::Quaterniond Rotation(WorldRotation->W, WorldRotation->X, WorldRotation->Y, WorldRotation->Z);
    Eigen::Vector3d Translation(WorldPosition->X, WorldPosition->Y, WorldPosition->Z);
    
    Eigen::Transform<double, 3, Eigen::Affine> Transform = Eigen::Transform<double, 3, Eigen::Affine>::Identity();
    Transform.scale(Scale);
    Transform.rotate(Rotation);
    Transform.translate(Translation);
    
    for (int i = 0; i < VerticesLength; i++)
    {
        Eigen::Vector3d Vector (Vertices[i].X, Vertices[i].Y, Vertices[i].Z);
        Eigen::Vector3d TransformedVector = Transform * Vector;
        
        EigenVertices(i, 0) = TransformedVector(0, 0);
        EigenVertices(i, 1) = TransformedVector(1, 0);
        EigenVertices(i, 2) = -TransformedVector(2, 0);
    }
    
    // Copy indicies into a matrix
    IndiceMatrix EigenIndices (IndicesLength / 3, 3);
    
    int row, i;
    row = i = 0;
    
    for ( ; (row < IndicesLength / 3) && (i < IndicesLength);  row++, i+=3)
    {
        EigenIndices(row, 0) = Indices[i];
        EigenIndices(row, 1) = Indices[i+1];
        EigenIndices(row, 2) = Indices[i+2];
    }
    
    // Create a mesh
    PL_MESH Mesh;
    Mesh.Vertices = EigenVertices;
    Mesh.Indices = EigenIndices;
    
    // Add mesh to scene
    int Index = -1;
    PL_RESULT Result = Scene->AddMesh(Mesh, Index);
    *OutIndex = Index;
    return Result;
}

PL_RESULT PL_Scene_RemoveMesh(PL_SCENE* Scene, int IndexToRemove)
{
    if (!Scene || IndexToRemove < 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->RemoveMesh(IndexToRemove);
}

PL_RESULT PL_Scene_Voxelise(PL_SCENE* Scene, PLVector* CenterPosition, PLVector* Size)
{
    if (!Scene || !CenterPosition || !Size)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    // Create AABB
    const PLVector Min = *CenterPosition - *Size / 2;
    const PLVector Max = *CenterPosition + *Size / 2;
    
    Eigen::Vector3d EigenMin;
    Eigen::Vector3d EigenMax;
    EigenMin << (Eigen::Vector3d() << Min.X, Min.Y, Min.Z).finished();
    EigenMax << (Eigen::Vector3d() << Max.X, Max.Y, Max.Z).finished();
    
    Eigen::AlignedBox<double,3> Bounds = Eigen::AlignedBox<double,3>(EigenMin, EigenMax);
    
    // TODO: Change voxel size to something set by the user per simulation
    const float VoxelSize = 5.f;
    const int VoxelsInSide = static_cast<int>(Size->X / VoxelSize);
    
    Debug((std::string("Voxel Size: ") + std::to_string(VoxelSize)).c_str(), PL_DEBUG_LEVEL_LOG);
    Debug((std::string("Voxels Per Side: ") + std::to_string(VoxelsInSide)).c_str(), PL_DEBUG_LEVEL_LOG);
    
    // Matrix<Scalar, Rows, Columns, Options, MaxRows, MaxColumns>
    Eigen::Matrix<double, -1, -1, 0, -1, -1> CenterPositions;
    Eigen::MatrixXi Side;
    
    // Calculates all center positions of a voxel lattice within a bounding box
    igl::voxel_grid(Bounds, VoxelsInSide, 0, CenterPositions, Side);
    
    const int XSize = Side(0,0);
    const int YSize = Side(0,1);
    const int ZSize = Side(0,2);
    
    Debug((std::string("X Voxels: ") + std::to_string(XSize)).c_str(), PL_DEBUG_LEVEL_LOG);
    Debug((std::string("Y Voxels: ") + std::to_string(YSize)).c_str(), PL_DEBUG_LEVEL_LOG);
    Debug((std::string("Z Voxels: ") + std::to_string(ZSize)).c_str(), PL_DEBUG_LEVEL_LOG);
    
    for (int x = 0; x < XSize; x++)
    {
        for (int y = 0; y < YSize; y++)
        {
            for (int z = 0; z < ZSize; z++)
            {
                // The X,Y and Z sides of the lattice are all combined into the rows of the matrix
                // To access the correct index, we have to convert 3D index to a 1D index
                // https://stackoverflow.com/questions/16790584/converting-index-of-one-dimensional-array-into-two-dimensional-array-i-e-row-a#16790720
                // The actual vertex positions of the index are contained within 3 columns
                const int Index = x + y * XSize + z * XSize * YSize;
                const double XCor = CenterPositions(Index,0);
                const double YCor = CenterPositions(Index,1);
                const double ZCor = CenterPositions(Index,2);
                char FormatString[] = "X: %s. Y: %s. Z: %s";
                Debug((boost::format(FormatString) % std::to_string(XCor) % std::to_string(YCor) % std::to_string(ZCor)).str().c_str(), PL_DEBUG_LEVEL_LOG);
            }
        }
    }
    
    PL_VOXEL_GRID VoxelGrid;
    VoxelGrid.Bounds = Bounds;
    VoxelGrid.CenterPositions = CenterPositions;
    VoxelGrid.Size = Side;
    VoxelGrid.VoxelSize = VoxelSize;
    VoxelGrid.Voxels = std::vector<PLVoxel>(XSize*YSize*ZSize); // TODO: Actually fill the voxels with values
    
    Scene->SetVoxels(VoxelGrid);
    
    return PL_OK;
}

PL_RESULT PL_Scene_Debug(PL_SCENE* Scene)
{
    return Scene->OpenOpenGLDebugWindow();
}
