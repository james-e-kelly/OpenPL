/*
  ==============================================================================

    OpenPL.cpp
    Created: 15 Aug 2020 8:12:46pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/OpenPL.h"
#include <forward_list>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/voxel_grid.h>

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

/**
 * The system class is a container for scene objects. It's primary purpose is to remove having static variables inside this file.
 * Users have to create a system object before going further in the simulation.
 * And because the user has to release the object, it is a nice place to handle memory management.
 */
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

/**
 * The scene class is the main work horse of the simulation.
 * 1) Take mesh data from game and convert to internal data types
 * 2) Convert to voxels
 * 3) Simulate over voxels
 * 4) Return simulated data/parameters
 * 5) Pronably save to disk as well
 */
class PL_SCENE
{
public:
    
    /**
     * Constructor. Scene objects (and any other heap allocated objects) must reference the system object.
     * @param System System object that owns the scene.
     */
    PL_SCENE(PL_SYSTEM* System)
    :   OwningSystem(System)
    { }
    
    /**
     * Get the owning scene object.
     */
    PL_RESULT GetSystem(PL_SYSTEM** OutSystem) const
    {
        *OutSystem = OwningSystem;
        return PL_OK;
    }
    
    /**
     * Takes generic mesh data from the game, converts it to internal data and stores it within the scene.
     *
     * @param Scene Scene to add the mesh to.
     * @param WorldPosition World Position of the mesh. Used to offset the vertices' position.
     * @param WorldRotation World Rotation of the mesh. Used to rotate the vertices to reflect its rotation in the game.
     * @param WorldScale World Scale of the mesh. Used to scale the vertices to reflect their size in the game.
     * @param Vertices Pointer to the start of a vertices array.
     * @param VerticesLength Length of the vertices array.
     * @param Indices Pointer to the start of an indices array.
     * @param IndicesLength Length of the indicies array.
     * @param OutIndex If successful, the index the mesh is stored at for later deletion.
     */
    PL_RESULT AddAndConvertGameMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex)
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
        VertexMatrix EigenVertices (VerticesLength, 3); // Eigen::Matrix(Index rows, Index columns);
        
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
        PL_RESULT Result = AddMesh(Mesh, Index);
        *OutIndex = Index;
        return Result;
    }
    
    /**
     * Add a PL_MESH to the scene's internal storage.
     *
     * @param Mesh Mesh to add.
     * @param OutIndex Index of the mesh within the array.
     */
    PL_RESULT AddMesh(PL_MESH& Mesh, int& OutIndex)
    {
        Meshes.push_back(Mesh);
        OutIndex = static_cast<int>(Meshes.size()) - 1;
        return PL_OK;
    }
    
    /**
     * Removes a PL_MESH from the internal array using its index.
     *
     * @param Index Index to remove.
     */
    PL_RESULT RemoveMesh(int Index)
    {
        Meshes.erase(Meshes.begin()+Index);
        return PL_OK;
    }
    
    /**
     * Quick internal debugging method for displaying the meshes within the scene.
     */
    PL_RESULT OpenOpenGLDebugWindow() const
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
    
    /**
     * Converts the scene's geometry to voxels, ready for simulation.
     *
     * @param CenterPosition Center of the AABB containing all the voxels.
     * @param Size Size of the voxel lattice.
     * @param VoxelSize Size of each voxel cell.
     */
    PL_RESULT Voxelise(PLVector CenterPosition, PLVector Size, float VoxelSize = 5.f)
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
        if (XSize == 0 || YSize == 0 || ZSize)
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
    
    /**
     * Adds absorption values to the voxel lattice cells based on the absorptivity of each mesh.
     *
     * If Voxelise creates the voxels, this method gives them meaning.
     */
    PL_RESULT FillVoxels()
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
            Eigen::Vector3d MeshMin = Mesh.Vertices.colwise().minCoeff();
            Eigen::Vector3d MeshMax = Mesh.Vertices.colwise().maxCoeff();
            Eigen::AlignedBox<double, 3> MeshBounds (MeshMin, MeshMax);
            
            // Ignore mesh if it's not within the lattice
            if (!Voxels.Bounds.contains(MeshBounds))
            {
                continue;
            }
            
            // List of all cells that fit within the mesh
            std::vector<PLVoxel*> MeshCells;
            
            // Vector3 of each voxel size
            Eigen::Vector3d VoxelSize;
            VoxelSize << Voxels.VoxelSize, Voxels.VoxelSize, Voxels.VoxelSize;
            
            // For each voxel in the lattice, find if it's within the mesh bounds
            // If it is, add it to the list
            for (auto& Cell : Voxels.Voxels)
            {
                Eigen::Vector3d Pos = Cell.WorldPosition;
                Eigen::Vector3d Min = Pos - (VoxelSize / 2);
                Eigen::Vector3d Max = Pos + (VoxelSize / 2);
                
                Eigen::AlignedBox<double, 3> VoxelBounds (Min,Max);
                
                if (MeshBounds.contains(VoxelBounds))
                {
                    MeshCells.push_back(&Cell);
                }
            }
            
            // Somehow there are no voxels for this mesh, even though it's inside the lattice
            if (MeshCells.size() == 0)
            {
                DebugWarn("Couldn't find voxels for a mesh. This shouldn't be possible");
                continue;
            }
            
            for (int i = 0; i < Mesh.Indices.size(); i++)
            {
                const int Indice1 = Mesh.Indices(i,0);
                const int Indice2 = Mesh.Indices(i,1);
                const int Indice3 = Mesh.Indices(i,2);
                
                Eigen::Matrix<double,3,3,0,3,3> VertexPositions;
                VertexPositions <<  Mesh.Vertices(Indice1,0), Mesh.Vertices(Indice1,1), Mesh.Vertices(Indice1,2),
                                    Mesh.Vertices(Indice2,0), Mesh.Vertices(Indice2,1), Mesh.Vertices(Indice3,2),
                                    Mesh.Vertices(Indice3,0), Mesh.Vertices(Indice3,1), Mesh.Vertices(Indice3,2);
                
                Eigen::Vector3d Min = VertexPositions.colwise().minCoeff();
                Eigen::Vector3d Max = VertexPositions.colwise().maxCoeff();
                Eigen::AlignedBox<double, 3> FaceBounds (Min, Max);
                
                for (auto& Cell : MeshCells)
                {
                    Eigen::Vector3d Pos = Cell->WorldPosition;
                    Eigen::Vector3d Min = Pos - (VoxelSize / 2);
                    Eigen::Vector3d Max = Pos + (VoxelSize / 2);
                    
                    Eigen::AlignedBox<double, 3> VoxelBounds (Min,Max);
                    
                    if (FaceBounds.contains(VoxelBounds))
                    {
                        Cell->Absorptivity = 0.75f; // DEBUG. TODO: Fill this with an actual value
                    }
                }
            }
        }
        
        return PL_OK;
    }
    
private:
    PL_SYSTEM* OwningSystem;
    
    std::vector<PL_MESH> Meshes;
    
    PL_VOXEL_GRID Voxels;
};

PL_RESULT PL_Debug_Initialize (PL_Debug_Callback Callback)
{
    DebugCallback = Callback;
    return PL_OK;
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
    if (!Scene || !WorldPosition || !WorldRotation || !WorldScale)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->AddAndConvertGameMesh(*WorldPosition, *WorldRotation, *WorldScale, Vertices, VerticesLength, Indices, IndicesLength, OutIndex);
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
    
    return Scene->Voxelise(*CenterPosition, *Size);
}

PL_RESULT PL_Scene_Debug(PL_SCENE* Scene)
{
    return Scene->OpenOpenGLDebugWindow();
}
