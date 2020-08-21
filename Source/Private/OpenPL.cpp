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
    
private:
    PL_SYSTEM* OwningSystem;
    
    std::vector<PL_MESH> Meshes;
};

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
    if (!System || OutScene)
    {
        return PL_ERR_MEMORY;
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

PL_RESULT PL_Scene_AddMesh(PL_SCENE* Scene, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex)
{
    // Return if any pointers are invalid
    if (!Scene || !Vertices || !Indices)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    // Indices must be multiple of 3
    if (IndicesLength % 3 != 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    // Copy vertices into a matrix
    VertexMatrix EigenVertices (VerticesLength, 3); // Eigen::Matrix(Index rows, Index columns);
    
    for (int i = 0; i < VerticesLength; i++)
    {
        EigenVertices(i, 0) = Vertices[i].X;
        EigenVertices(i, 1) = Vertices[i].Y;
        EigenVertices(i, 2) = Vertices[i].Z;
    }
    
    // Copy indicies into a matrix
    IndiceMatrix EigenIndices (IndicesLength / 3, 3);
    
    for (int i = 0; i < IndicesLength; i+=3)
    {
        EigenIndices(i, 0) = Indices[i];
        EigenIndices(i, 1) = Indices[i+1];
        EigenIndices(i, 2) = Indices[i+2];
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
    std::vector<PL_MESH>& Meshes = Scene->GetMeshes();
    
    int VertexRows = 0;
    int IndiceRows = 0;
    
    // Loop once to get full row size
    // Doing a double loop means less reallocating
    for (auto& Mesh : Meshes)
    {
        VertexRows += Mesh.Vertices.rows();
        IndiceRows += Mesh.Indices.rows();
    }
    
    VertexMatrix V(VertexRows, 3);
    IndiceMatrix F(IndiceRows, 3);
    
    for(auto& Mesh : Meshes)
    {
        V << Mesh.Vertices;
        F << Mesh.Indices;
    }

    igl::opengl::glfw::Viewer viewer;
    viewer.data().set_mesh(V, F);
    viewer.launch();
    
    return PL_OK;
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
