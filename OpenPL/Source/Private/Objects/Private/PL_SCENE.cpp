/*
  ==============================================================================

    PL_SCENE.cpp
    Created: 3 Feb 2021 2:19:58pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/PL_SCENE.h"
#include "../Public/PL_SYSTEM.h"
#include <igl/opengl/glfw/Viewer.h>
#include <igl/voxel_grid.h>
#include <sstream>

PL_SCENE::PL_SCENE(PL_SYSTEM* System)
:   OwningSystem(System)
{ }

PL_RESULT PL_SCENE::GetSystem(PL_SYSTEM** OutSystem) const
{
    *OutSystem = OwningSystem;
    return PL_OK;
}

PL_RESULT PL_SCENE::AddAndConvertGameMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex)
{
    // Return if any pointers are invalid
    if (!Vertices || !Indices)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    if (VerticesLength <= 3)
    {
        DebugError("Can't create geometry from a mesh that has less than 3 vertices");
        return PL_ERR_INVALID_PARAM;
    }
    
    if (IndicesLength <= 3)
    {
        DebugError("Can't create geometry from a mesh that has less than 3 indices");
        return PL_ERR_INVALID_PARAM;
    }
    
    // Indices must be multiple of 3
    if (IndicesLength % 3 != 0)
    {
        DebugError("Can't create mesh. Indices wasn't a multiple of 3");
        return PL_ERR_INVALID_PARAM;
    }
    
    // Copy vertices into a matrix
    VertexMatrix EigenVertices(3, VerticesLength); // Eigen::Matrix(Index rows, Index columns);
    
    Eigen::Vector3d Scale (WorldScale.X, WorldScale.Y, WorldScale.Z);
    Eigen::Quaterniond Rotation(WorldRotation.W, WorldRotation.X, WorldRotation.Y, WorldRotation.Z);
    Eigen::Vector3d Translation(WorldPosition.X, WorldPosition.Y, WorldPosition.Z);
    
    Eigen::Transform<double, 3, Eigen::Affine> Transform = Eigen::Transform<double, 3, Eigen::Affine>::Identity();
    Transform.scale(Scale);
    Transform.rotate(Rotation);
    Transform.translate(Translation);
    
    for (int i = 0; i < VerticesLength; i++)
    {
        Eigen::Vector3d Vector (Vertices[i].X, Vertices[i].Y, Vertices[i].Z);
        
        EigenVertices(0,i) = Vector(0, 0);
        EigenVertices(1,i) = Vector(1, 0);
        EigenVertices(2,i) = Vector(2, 0);
//
//        EigenVertices(i, 0) = TransformedVector(0, 0);
//        EigenVertices(i, 1) = TransformedVector(1, 0);
//        EigenVertices(i, 2) = TransformedVector(2, 0);
    }
    
    VertexMatrix TransformedVertices = Transform * EigenVertices.colwise().homogeneous();
    
    // Copy indicies into a matrix
    IndiceMatrix EigenIndices (3, IndicesLength / 3);
    
    int row, i;
    row = i = 0;
    
    for ( ; (row < IndicesLength / 3) && (i < IndicesLength);  row++, i+=3)
    {
        EigenIndices(0, row) = Indices[i];
        EigenIndices(1, row) = Indices[i+1];
        EigenIndices(2, row) = Indices[i+2];
    }
    
    // Create a mesh
    PL_MESH Mesh;
    Mesh.Vertices = TransformedVertices;
    Mesh.Indices = EigenIndices;
    
    // Add mesh to scene
    int Index = -1;
    PL_RESULT Result = AddMesh(Mesh, Index);
    *OutIndex = Index;
    return Result;
}

PL_RESULT PL_SCENE::AddMesh(PL_MESH& Mesh, int& OutIndex)
{
    Meshes.push_back(Mesh);
    OutIndex = static_cast<int>(Meshes.size()) - 1;
    return PL_OK;
}

PL_RESULT PL_SCENE::RemoveMesh(int Index)
{
    Meshes.erase(Meshes.begin()+Index);
    return PL_OK;
}

PL_RESULT PL_SCENE::AddListenerLocation(PLVector& Location, int& OutIndex)
{
    ListenerLocations.push_back(Location);
    OutIndex = static_cast<int>(ListenerLocations.size()) - 1;
    return PL_OK;
}

PL_RESULT PL_SCENE::RemoveListenerLocation(int Index)
{
    ListenerLocations.erase(ListenerLocations.begin()+Index);
    return PL_OK;
}

PL_RESULT PL_SCENE::AddSourceLocation(PLVector& Location, int& OutIndex)
{
    SourceLocations.push_back(Location);
    OutIndex = static_cast<int>(SourceLocations.size()) - 1;
    return PL_OK;
}

PL_RESULT PL_SCENE::RemoveSourceLocation(int Index)
{
    SourceLocations.erase(SourceLocations.begin()+Index);
    return PL_OK;
}

PL_RESULT PL_SCENE::OpenOpenGLDebugWindow() const
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

PL_RESULT PL_SCENE::Voxelise(PLVector CenterPosition, PLVector Size, float VoxelSize)
{
    if (Meshes.size() == 0)
    {
        return PL_ERR;
    }
    
    if (Size.X < VoxelSize || Size.Y < VoxelSize || Size.Z < VoxelSize)
    {
        DebugWarn("Can't create voxel lattice of that size. No voxels would fit inside it");
        return PL_ERR_INVALID_PARAM;
    }
    
    // Create AABB
    const PLVector Min = CenterPosition - Size / 2;
    const PLVector Max = CenterPosition + Size / 2;
    
    Eigen::Vector3d EigenMin;
    Eigen::Vector3d EigenMax;
    EigenMin << (Eigen::Vector3d() << Min.X, Min.Y, Min.Z).finished();
    EigenMax << (Eigen::Vector3d() << Max.X, Max.Y, Max.Z).finished();
    
    Eigen::AlignedBox<double,3> Bounds = Eigen::AlignedBox<double,3>(EigenMin, EigenMax);
    
    // TODO: Change voxel size to something set by the user per simulation
    const int VoxelsInSide = static_cast<int>(Size.X / VoxelSize);
    
    // Matrix<Scalar, Rows, Columns, Options, MaxRows, MaxColumns>
    Eigen::Matrix<double, -1, -1, 0, -1, -1> CenterPositions;
    Eigen::MatrixXi Side;
    
    // Calculates all center positions of a voxel lattice within a bounding box
    igl::voxel_grid(Bounds, VoxelsInSide, 0, CenterPositions, Side);
    
    const int XSize = Side(0,0);
    const int YSize = Side(0,1);
    const int ZSize = Side(0,2);
    
    // We can assume if any sides of the lattice are 0, something went wrong
    // Even if we wanted a 2D grid, we'd still need at least 1 along the Z
    if (XSize == 0 || YSize == 0 || ZSize == 0)
    {
        DebugError("Failed to create voxels");
        return PL_ERR;
    }
    
    // Set voxels
    PL_VOXEL_GRID VoxelGrid;
    VoxelGrid.Bounds = Bounds;
    VoxelGrid.Size = Side;
    VoxelGrid.VoxelSize = VoxelSize;
    VoxelGrid.Voxels = std::vector<PLVoxel>(XSize*YSize*ZSize); // TODO: Actually fill the voxels with values
    
    // Set positions for each voxel
    for (int X = 0; X < XSize; X++)
    {
        for (int Y = 0; Y < YSize; Y++)
        {
            for (int Z = 0; Z < ZSize; Z++)
            {
                const int Index = ThreeDimToOneDim(X, Y, Z, XSize, YSize);
                const double XCor = CenterPositions(Index, 0);
                const double YCor = CenterPositions(Index, 1);
                const double ZCor = CenterPositions(Index, 2);
                Eigen::Vector3d WorldPosition;
                WorldPosition << XCor, YCor, ZCor;
                VoxelGrid.Voxels[Index].WorldPosition = WorldPosition;
            }
        }
    }
    
    Voxels = VoxelGrid;
    
    return FillVoxels();
}

PL_RESULT PL_SCENE::FillVoxels()
{
    // First thought on how to do this:
    // Create an AABB for each mesh
    // Find all voxel cells that fit within the box
    // Interate over each face and find the bounding box of that face
    // Find the cells which fit within the face AABB
    // Populate those cells with absorption values
    
    // It's probably more accurate to shoot a ray between each vertex
    // However, at the sizes of the voxels and faces, this shouldn't be too much of a problem
    // But if accuracy does become a problem, I think that will be the solution
    
    for (auto& Mesh : Meshes)
    {
        // Full AABB that encloses the mesh
        Eigen::Vector3d MeshMin = Mesh.Vertices.rowwise().minCoeff();
        Eigen::Vector3d MeshMax = Mesh.Vertices.rowwise().maxCoeff();
        Eigen::AlignedBox<double, 3> MeshBounds (MeshMin, MeshMax);
        
        // Ignore mesh if it's not within the lattice
        if (!Voxels.Bounds.intersects(MeshBounds))
        {
            continue;
        }
        
        // List of all cells that fit within the mesh
        std::vector<PLVoxel*> MeshCells;
        
        // Vector3 of each voxel size
        Eigen::Vector3d VoxelSize (Voxels.VoxelSize, Voxels.VoxelSize, Voxels.VoxelSize);
        
        // For each voxel in the lattice, find if it's within the mesh bounds
        // If it is, add it to the list
        for (auto& Cell : Voxels.Voxels)
        {
            Eigen::Vector3d Pos (Cell.WorldPosition);
            Eigen::Vector3d Min = Pos - (VoxelSize / 2);
            Eigen::Vector3d Max = Pos + (VoxelSize / 2);
            
            Eigen::AlignedBox<double, 3> VoxelBounds (Min,Max);
            
            if (MeshBounds.intersects(VoxelBounds))
            {
                MeshCells.push_back(&Cell);
            }
            else  if (VoxelBounds.intersects(MeshBounds))
            {
                DebugWarn("Voxel wasn't within the mesh, but the mesh is within the voxel");
                MeshCells.push_back(&Cell);
            }
        }
        
        // Somehow there are no voxels for this mesh, even though it's inside the lattice
        if (MeshCells.size() == 0)
        {
            DebugWarn("Couldn't find voxels for a mesh. This shouldn't be possible");
            continue;
        }
        
        for (int i = 0; i < Mesh.Indices.cols(); i++)
        {
            const int Indice1 = Mesh.Indices(0,i);
            const int Indice2 = Mesh.Indices(1,i);
            const int Indice3 = Mesh.Indices(2,i);
            
            Eigen::Matrix<double,3,3,0,3,3> VertexPositions;
            VertexPositions <<  Mesh.Vertices(0,Indice1), Mesh.Vertices(1,Indice1), Mesh.Vertices(2,Indice1),
            Mesh.Vertices(0,Indice2), Mesh.Vertices(1,Indice2), Mesh.Vertices(2,Indice2),
            Mesh.Vertices(0,Indice3), Mesh.Vertices(1,Indice3), Mesh.Vertices(2,Indice3);
            
            Eigen::Vector3d Min = VertexPositions.rowwise().minCoeff();
            Eigen::Vector3d Max = VertexPositions.rowwise().maxCoeff();
            Eigen::AlignedBox<double, 3> FaceBounds (Min, Max);
            
            for (auto& Cell : MeshCells)
            {
                Eigen::Vector3d Pos = Cell->WorldPosition;
                Eigen::Vector3d Min = Pos - (VoxelSize / 2);
                Eigen::Vector3d Max = Pos + (VoxelSize / 2);
                
                Eigen::AlignedBox<double, 3> VoxelBounds (Min,Max);
                
                if (FaceBounds.intersects(VoxelBounds))
                {
                    Cell->Absorptivity = 0.75f; // DEBUG. TODO: Fill this with an actual value
                }
                else if (VoxelBounds.intersects(FaceBounds))
                {
                    DebugWarn("Face didn't contain voxel, but voxel contained the face");
                    Cell->Absorptivity = 0.75f; // DEBUG. TODO: Fill this with an actual value
                }
            }
        }
    }
    
    return PL_OK;
}

PL_RESULT PL_SCENE::GetVoxelsCount(int* OutVoxelCount)
{
    if (!OutVoxelCount)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    *OutVoxelCount = static_cast<int>(Voxels.Voxels.size());
    
    return PL_OK;
}

PL_RESULT PL_SCENE::GetVoxelLocation(PLVector* OutVoxelLocation, int Index)
{
    if (!OutVoxelLocation || Index < 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    if (Index >= Voxels.Voxels.size())
    {
        return PL_ERR;  // Should change this to a warning or something similar
    }
    
    const Eigen::Vector3d* VectorLocation = &Voxels.Voxels[Index].WorldPosition;
    OutVoxelLocation->X = VectorLocation->x();
    OutVoxelLocation->Y = VectorLocation->y();
    OutVoxelLocation->Z = VectorLocation->z();
    
    // WARNING:
    // There was previously code here that exposed a bug
    // *OutVoxelLocation = SomeOtherVector
    // This wouldn't copy the Z component over
    // No clue as to why
    
    return PL_OK;
}

PL_RESULT PL_SCENE::GetVoxelAbsorpivity(float* OutAbsorpivity, int Index)
{
    if (!OutAbsorpivity || Index < 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    if (Index >= Voxels.Voxels.size())
    {
        return PL_ERR;  // Should change this to a warning or something similar
    }
    
    *OutAbsorpivity = Voxels.Voxels[Index].Absorptivity;
    
    return PL_OK;
}
