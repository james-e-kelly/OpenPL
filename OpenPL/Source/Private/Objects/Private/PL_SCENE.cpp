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
#include <igl/copyleft/cgal/points_inside_component.h>
#include <sstream>
#include "../Public/MatPlotPlotter.h"
#include "../Public/Simulators/SimulatorFDTD.h"
#include "../Public/Simulators/SimulatorBasic.h"
#include <boost/timer/timer.hpp>

PL_SCENE::PL_SCENE(PL_SYSTEM* System)
:   OwningSystem(System)
{ }

PL_SCENE::~PL_SCENE()
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
    Transform.rotate(Rotation).translate(Translation).scale(Scale);
    
    for (int i = 0; i < VerticesLength; i++)
    {
        Eigen::Vector3d Vector (Vertices[i].X, Vertices[i].Y, Vertices[i].Z);
        
        EigenVertices(0,i) = Vector(0, 0);
        EigenVertices(1,i) = Vector(1, 0);
        EigenVertices(2,i) = Vector(2, 0);
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
    if (Index >= ListenerLocations.size())
    {
        DebugError("Index out of bounds when removing listener");
        return PL_ERR;
    }
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
    if (Index >= SourceLocations.size())
    {
        DebugError("Index out of bounds when removing emitter");
        return PL_ERR;
    }
    SourceLocations.erase(SourceLocations.begin()+Index);
    return PL_OK;
}

PL_RESULT PL_SCENE::OpenOpenGLDebugWindow() const
{
    igl::opengl::glfw::Viewer viewer;
    for (auto& Mesh : Meshes)
    {
        // Transpose the matrices because we store vectors as:
        // {x1, x2, x3}
        // {y1, y2, y3}
        // {z1, z2, z3}
        // However, OpenGL expects:
        // {x1, y1, z1}
        // {x2, y2, z2}
        // {x3, y3, z3}
        viewer.data().set_mesh(Mesh.Vertices.transpose(), Mesh.Indices.transpose());
        viewer.append_mesh(true);
        
        Eigen::Vector3d MeshMin = Mesh.Vertices.rowwise().minCoeff();
        Eigen::Vector3d MeshMax = Mesh.Vertices.rowwise().maxCoeff();
        Eigen::AlignedBox<double, 3> MeshBounds (MeshMin, MeshMax);
        
        VertexMatrix BoundingBoxPoints(8,3);
        BoundingBoxPoints <<
        MeshMin(0), MeshMin(1), MeshMin(2),
        MeshMax(0), MeshMin(1), MeshMin(2),
        MeshMax(0), MeshMax(1), MeshMin(2),
        MeshMin(0), MeshMax(1), MeshMin(2),
        MeshMin(0), MeshMin(1), MeshMax(2),
        MeshMax(0), MeshMin(1), MeshMax(2),
        MeshMax(0), MeshMax(1), MeshMax(2),
        MeshMin(0), MeshMax(1), MeshMax(2);
        
        // Edges of the bounding box
        IndiceMatrix E_box(12,2);
        E_box <<
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        7 ,3;
        
        viewer.data().add_points(BoundingBoxPoints,Eigen::RowVector3d(1,0,0));

        // Plot the edges of the bounding box
        for (unsigned i=0;i<E_box.rows(); ++i)
          viewer.data().add_edges
          (
           BoundingBoxPoints.row(E_box(i,0)),
           BoundingBoxPoints.row(E_box(i,1)),
            Eigen::RowVector3d(1,0,0)
          );
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
    
    PL_RESULT ReturnResult = PL_ERR;
    
    switch (VoxelThreadStatus.load())
    {
        case ThreadStatus_NotStarted:
        {
            // Scoped Threads are RAII
            // Therefore, you can't do `scoped_thread thread = otherThread;`
            // However, scoped threads can be moved
            // So create an object on the stack, then move ownership to the class' member variable
            boost::scoped_thread<> NewVoxelThread(boost::thread(&PL_SCENE::VoxeliseInternal, this, CenterPosition, Size, VoxelSize));
            VoxelThread = std::move(NewVoxelThread);
            ReturnResult = PL_OK;
            break;
        }
        case ThreadStatus_Ongoing:
        {
            ReturnResult = PL_OK;
            break;
        }
        case ThreadStatus_Finished:
        {
            if (VoxelThread.joinable())
            {
                VoxelThread.join();
            }
            
            VoxelThreadStatus.store(ThreadStatus_NotStarted);
            ReturnResult = PL_OK;
            break;
        }
    }
    return ReturnResult;
}

VertexMatrix GetPointsToCheckForVoxel(Eigen::Vector3d VoxelPosition, Eigen::Vector3d VoxelSize)
{
    VertexMatrix PointsToCheck(9,3);
    double HalfSize = VoxelSize.x() / 2;
    
    // Center
    PointsToCheck(0,0) = VoxelPosition.x();
    PointsToCheck(0,1) = VoxelPosition.y();
    PointsToCheck(0,2) = VoxelPosition.z();
    
    // Front, top, left
    PointsToCheck(1,0) = VoxelPosition.x() + HalfSize;
    PointsToCheck(1,1) = VoxelPosition.y() + HalfSize;
    PointsToCheck(1,2) = VoxelPosition.z() - HalfSize;
    
    // Front, top, right
    PointsToCheck(2,0) = VoxelPosition.x() + HalfSize;
    PointsToCheck(2,1) = VoxelPosition.y() + HalfSize;
    PointsToCheck(2,2) = VoxelPosition.z() + HalfSize;
    
    // Back, top, left
    PointsToCheck(3,0) = VoxelPosition.x() - HalfSize;
    PointsToCheck(3,1) = VoxelPosition.y() + HalfSize;
    PointsToCheck(3,2) = VoxelPosition.z() - HalfSize;
    
    // Back, top, right
    PointsToCheck(4,0) = VoxelPosition.x() - HalfSize;
    PointsToCheck(4,1) = VoxelPosition.y() + HalfSize;
    PointsToCheck(4,2) = VoxelPosition.z() + HalfSize;
    
    // Front, bottom, left
    PointsToCheck(5,0) = VoxelPosition.x() + HalfSize;
    PointsToCheck(5,1) = VoxelPosition.y() - HalfSize;
    PointsToCheck(5,2) = VoxelPosition.z() - HalfSize;
    
    // Front, bottom, right
    PointsToCheck(6,0) = VoxelPosition.x() + HalfSize;
    PointsToCheck(6,1) = VoxelPosition.y() - HalfSize;
    PointsToCheck(6,2) = VoxelPosition.z() + HalfSize;
    
    // Back, bottom, left
    PointsToCheck(7,0) = VoxelPosition.x() - HalfSize;
    PointsToCheck(7,1) = VoxelPosition.y() - HalfSize;
    PointsToCheck(7,2) = VoxelPosition.z() - HalfSize;
    
    // Back, bottom, right
    PointsToCheck(8,0) = VoxelPosition.x() - HalfSize;
    PointsToCheck(8,1) = VoxelPosition.y() - HalfSize;
    PointsToCheck(8,2) = VoxelPosition.z() + HalfSize;
    
    return PointsToCheck;
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
    
    // First, init all Beta fields to 1
    // Ie, to open air
    
    for (auto& Voxel : Voxels.Voxels)
    {
        Voxel.Beta = 1;
    }
    
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
        
        VertexMatrix TransposedVertices = Mesh.Vertices.transpose();
        IndiceMatrix TransposedIndices = Mesh.Indices.transpose();
        
        for (auto& MeshCell : MeshCells)
        {
            VertexMatrix PointsToCheck = GetPointsToCheckForVoxel(MeshCell->WorldPosition, VoxelSize);
            IndiceMatrix ReturnPointsInside;
            
            igl::copyleft::cgal::points_inside_component(TransposedVertices, TransposedIndices, PointsToCheck, ReturnPointsInside);

            int NumberOfPointsInside = 0;
            
            for (int i = 0; i < ReturnPointsInside.size(); i++)
            {
                bool PointIsInside = ReturnPointsInside.data()[i] > 0;
                
                
                if (PointIsInside)
                {
                    NumberOfPointsInside++;
                }
            }
            
            if (NumberOfPointsInside > 2)
            {
                MeshCell->Absorptivity = 0.75f;
                MeshCell->Beta = 0;
            }
        }
    }
    
    VoxelThreadStatus.store(ThreadStatus_Finished);
    
    return PL_OK;
}

PL_RESULT PL_SCENE::Simulate()
{
    // Ignore for now so it's easier to test
//    if (Meshes.size() == 0 || ListenerLocations.size() == 0 || SourceLocations.size() == 0)
//    {
//        DebugWarn("Can't run simulation. Either need to provide geometry, emitter locations or listener locations");
//        return PL_ERR;
//    }
    
    // Yes, we're not multithreaded anymore but I need this to finish
    VoxelThread.join();
    
    Simulator = std::unique_ptr<class Simulator>(new SimulatorFDTD());
    
    PL_SIMULATION_SETTINGS Settings;
    Settings.Resolution = Low;
    Settings.TimeSteps = TimeSteps;
    
    Simulator->Init(Voxels, Settings);
    
    std::ostringstream Stream;
    Stream << "Simulating Over " << Voxels.Voxels.size() << " Voxels\n";
    {
        boost::timer::auto_cpu_timer SimulationTimer(Stream);
        Simulator->Simulate();
    }
    DebugLog(Stream.str().c_str());
    
    MatPlotPlotter plotter(Simulator->GetSimulatedLattice(), Voxels.Size(0,0), Voxels.Size(0,1), Voxels.Size(0,2), TimeSteps);
    plotter.PlotOneDimensionWaterfall();
    
    return PL_OK;
}

PL_RESULT PL_SCENE::GetVoxelsCount(int* OutVoxelCount)
{
    if (VoxelThreadStatus.load() == ThreadStatus_Ongoing)
    {
        *OutVoxelCount = 0;
        return PL_OK;
    }
    
    if (!OutVoxelCount)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    *OutVoxelCount = static_cast<int>(Voxels.Voxels.size());
    
    return PL_OK;
}

PL_RESULT PL_SCENE::GetVoxelLocation(PLVector* OutVoxelLocation, int Index)
{
    if (VoxelThreadStatus.load() == ThreadStatus_Ongoing)
    {
        *OutVoxelLocation = PLVector();
        return PL_OK;
    }
    
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
    
    if (VoxelThreadStatus.load() == ThreadStatus_Ongoing)
    {
        *OutAbsorpivity = 0.f;
        return PL_OK;
    }
    
    *OutAbsorpivity = Voxels.Voxels[Index].Absorptivity;
    
    return PL_OK;
}

PL_RESULT PL_SCENE::VoxeliseInternal(PLVector CenterPosition, PLVector Size, float VoxelSize)
{
    VoxelThreadStatus.store(ThreadStatus_Ongoing);
    
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
