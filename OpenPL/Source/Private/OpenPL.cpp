/*
  ==============================================================================

    OpenPL.cpp
    Created: 15 Aug 2020 8:12:46pm
    Author:  James Kelly

  ==============================================================================
*/

#include "../Public/OpenPL.h"
#include "../Private/Objects/Public/PL_SYSTEM.h"
#include "../Private/Objects/Public/PL_SCENE.h"

/**
 * Testing to try and create the wave equation.
 * I don't think this will be very needed though
 */
static void WaveEquation(Eigen::Vector3d Position, double Time, double PressureValue)
{
    // (∂² × p(x,t) / ∂ × t²) - c² × Laplacian × p(x,t) = F(x,t)
}

PL_RESULT PL_Debug_Initialize (PL_Debug_Callback Callback)
{
    SetDebugCallback(Callback);
    return PL_OK;
}

PL_RESULT PL_System_Create (PL_SYSTEM** OutSystem)
{
    if (!OutSystem)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    *OutSystem = new (std::nothrow) PL_SYSTEM();
    
    if (*OutSystem == nullptr)
    {
        return PL_ERR_MEMORY;
    }
    
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
    
    PL_SCENE* CreatedScene = new (std::nothrow) PL_SCENE(System);
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

PL_RESULT PL_Scene_Voxelise(PL_SCENE* Scene, PLVector* CenterPosition, PLVector* Size, float VoxelSize)
{
    if (!Scene || !CenterPosition || !Size)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->Voxelise(*CenterPosition, *Size, VoxelSize);
}

PL_RESULT PL_Scene_AddListenerLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex)
{
    if (!Scene || !Position || !OutIndex)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->AddListenerLocation(*Position, *OutIndex);
}

PL_RESULT PL_Scene_RemoveListenerLocation(PL_SCENE* Scene, int IndexToRemove)
{
    if (!Scene || IndexToRemove < 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->RemoveListenerLocation(IndexToRemove);
}

PL_RESULT PL_Scene_AddSourceLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex)
{
    if (!Scene || !Position || !OutIndex)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->AddSourceLocation(*Position, *OutIndex);
}

PL_RESULT PL_Scene_RemoveSourceLocation(PL_SCENE* Scene, int IndexToRemove)
{
    if (!Scene || IndexToRemove < 0)
    {
        return PL_ERR_INVALID_PARAM;
    }
    
    return Scene->RemoveSourceLocation(IndexToRemove);
}

PL_RESULT PL_Scene_Debug(PL_SCENE* Scene)
{
    return Scene->OpenOpenGLDebugWindow();
}

PL_RESULT PL_Scene_GetVoxelsCount(PL_SCENE* Scene, int* OutVoxelCount)
{
    return Scene->GetVoxelsCount(OutVoxelCount);
}

PL_RESULT PL_Scene_GetVoxelLocation(PL_SCENE* Scene, PLVector* OutVoxelLocation, int Index)
{
    return Scene->GetVoxelLocation(OutVoxelLocation, Index);
}

PL_RESULT PL_Scene_GetVoxelAbsorpivity(PL_SCENE* Scene, float* OutAbsorpivity, int Index)
{
    return Scene->GetVoxelAbsorpivity(OutAbsorpivity, Index);
}
