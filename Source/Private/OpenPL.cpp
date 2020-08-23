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
#include <igl/opengl/glfw/Viewer.h>

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
    
private:
    PL_SYSTEM* OwningSystem;
    
    std::vector<PL_MESH> Meshes;
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

PL_RESULT PL_Scene_Debug(PL_SCENE* Scene)
{
    return Scene->OpenOpenGLDebugWindow();
}

//PL_RESULT PLScene::GetBounds(PLBounds* OutBounds) const
//{
//    if (Impl)
//    {
//        *OutBounds = PLBounds::CreateAABB(Impl->SceneCenter, Impl->SceneSize);
//        return PL_OK;
//    }
//    return PL_ERR_MEMORY;
//}
