/*
  ==============================================================================

    OpenPL.cpp
    Created: 20 Aug 2020 4:30:43pm
    Author:  James Kelly

  ==============================================================================
*/

#include "OpenPL.hpp"

namespace OpenPL
{
    PL_RESULT PLSystem::Release()
    {
        return PL_System_Release(reinterpret_cast<PL_SYSTEM*>(this));
    }

    PL_RESULT PLSystem::SetListenerPosition(PLVector ListenerPosition)
    {
        return PL_System_SetListenerPosition(reinterpret_cast<PL_SYSTEM*>(this), ListenerPosition);
    }

    PL_RESULT PLSystem::GetListenerPositiion(PLVector *OutListenerPosition)
    {
        return PL_System_GetListenerPosition(reinterpret_cast<PL_SYSTEM*>(this), OutListenerPosition);
    }

    PL_RESULT PLSystem::CreateScene(PLScene** OutScene)
    {
        PL_SCENE* CreatedScene = nullptr;
        PL_RESULT Result = PL_System_CreateScene(reinterpret_cast<PL_SYSTEM*>(this), &CreatedScene);
        *OutScene = (PLScene*)CreatedScene;
        return Result;
    }

    PL_RESULT PLScene::Release()
    {
        return PL_Scene_Release(reinterpret_cast<PL_SCENE*>(this));
    }

    PL_RESULT PLScene::CreateVoxels(PLVector SceneSize, float VoxelSize)
    {
        return PL_Scene_CreateVoxels(reinterpret_cast<PL_SCENE*>(this), SceneSize, VoxelSize);
    }

    PL_RESULT PLScene::AddMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex)
    {
        return PL_Scene_AddMesh(reinterpret_cast<PL_SCENE*>(this), WorldPosition, WorldRotation, WorldScale, Vertices, VerticesLength, Indices, IndicesLength, OutIndex);
    }

    PL_RESULT PLScene::RemoveMesh(int IndexToRemove)
    {
        return PL_Scene_RemoveMesh(reinterpret_cast<PL_SCENE*>(this), IndexToRemove);
    }

    PL_RESULT PLScene::FillVoxelsWithGeometry()
    {
        return PL_Scene_FillVoxelsWithGeometry(reinterpret_cast<PL_SCENE*>(this));
    }

    PL_RESULT PLScene::AddListenerLocation(PLVector Position, int* OutIndex)
    {
        return PL_Scene_AddListenerLocation(reinterpret_cast<PL_SCENE*>(this), Position, OutIndex);
    }

    PL_RESULT PLScene::RemoveListenerLocation(int IndexToRemove)
    {
        return PL_Scene_RemoveListenerLocation(reinterpret_cast<PL_SCENE*>(this), IndexToRemove);
    }

    PL_RESULT PLScene::AddSourceLocation(PLVector Position, int* OutIndex)
    {
        return PL_Scene_AddSourceLocation(reinterpret_cast<PL_SCENE*>(this), Position, OutIndex);
    }

    PL_RESULT PLScene::RemoveSourceLocation(int IndexToRemove)
    {
        return PL_Scene_RemoveSourceLocation(reinterpret_cast<PL_SCENE*>(this), IndexToRemove);
    }

    PL_RESULT PLScene::Simulate(PLVector SimulationLocation)
    {
        return PL_Scene_Simulate(reinterpret_cast<PL_SCENE*>(this), SimulationLocation);
    }

    PL_RESULT PLScene::Debug()
    {
        return PL_Scene_Debug(reinterpret_cast<PL_SCENE*>(this));
    }

    PL_RESULT PLScene::GetVoxelsCount(int* OutVoxelCount)
    {
        return PL_Scene_GetVoxelsCount(reinterpret_cast<PL_SCENE*>(this), OutVoxelCount);
    }

    PL_RESULT PLScene::GetVoxelLocation(PLVector* OutVoxelLocation, int Index)
    {
        return PL_Scene_GetVoxelLocation(reinterpret_cast<PL_SCENE*>(this), OutVoxelLocation, Index);
    }

    PL_RESULT PLScene::GetVoxelAbsorpivity(float* OutAbsorpivity, int Index)
    {
        return PL_Scene_GetVoxelAbsorpivity(reinterpret_cast<PL_SCENE*>(this), OutAbsorpivity, Index);
    }

    PL_RESULT PLScene::DrawGraph(PLVector GraphPosition)
    {
        return PL_Scene_DrawGraph(reinterpret_cast<PL_SCENE*>(this), GraphPosition);
    }

    PL_RESULT PLScene::Encode(PLVector EncodingPosition, int* OutVoxelIndex)
    {
        return PL_Scene_Encode(reinterpret_cast<PL_SCENE*>(this), EncodingPosition, OutVoxelIndex);
    }
}
