/*
  ==============================================================================

    OpenPL_CPP.h
    Created: 20 Aug 2020 4:30:35pm
    Author:  James Kelly

  ==============================================================================
*/

/**
 * Header containing CPP objects for programming in CPP.
 */

#pragma once

#include "OpenPL.h"

namespace OpenPL
{
    class PLSystem;
    class PLScene;

    inline PL_RESULT Debug_Initialize(PL_Debug_Callback Callback) { return PL_Debug_Initialize(Callback); }
    inline PL_RESULT System_Create(PLSystem** OutSystem) { return PL_System_Create((PL_SYSTEM**)OutSystem); }

    /**
     * Handles object creation, memory management and baking.
     *
     * If any OpenPL object needs to be created, it should be made through the PLSystem.
     */
    class PLSystem
    {
    private:
        
        PLSystem();
        PLSystem(const PLSystem &);
        
    public:

        PL_RESULT JUCE_PUBLIC_FUNCTION Release();
        
        // Listener position
        PL_RESULT JUCE_PUBLIC_FUNCTION SetListenerPosition(PLVector ListenerPosition);
        PL_RESULT JUCE_PUBLIC_FUNCTION GetListenerPositiion(PLVector* OutListenerPosition);

        // Scenes
        PL_RESULT JUCE_PUBLIC_FUNCTION CreateScene(PLScene** OutScene);
    };

    class PLScene
    {
    private:
        
        PLScene();
        PLScene(const PLScene &);
        
    public:

        PL_RESULT JUCE_PUBLIC_FUNCTION Release();
        PL_RESULT JUCE_PUBLIC_FUNCTION CreateVoxels(PLVector SceneSize, float VoxelSize);
        PL_RESULT JUCE_PUBLIC_FUNCTION AddMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex);
        PL_RESULT JUCE_PUBLIC_FUNCTION RemoveMesh(int IndexToRemove);
        PL_RESULT JUCE_PUBLIC_FUNCTION FillVoxelsWithGeometry();
        PL_RESULT JUCE_PUBLIC_FUNCTION AddListenerLocation(PLVector Position, int* OutIndex);
        PL_RESULT JUCE_PUBLIC_FUNCTION RemoveListenerLocation(int IndexToRemove);
        PL_RESULT JUCE_PUBLIC_FUNCTION AddSourceLocation(PLVector Position, int* OutIndex);
        PL_RESULT JUCE_PUBLIC_FUNCTION RemoveSourceLocation(int IndexToRemove);
        PL_RESULT JUCE_PUBLIC_FUNCTION Simulate(PLVector SimulationLocation);
        PL_RESULT JUCE_PUBLIC_FUNCTION Debug();
        PL_RESULT JUCE_PUBLIC_FUNCTION GetVoxelsCount(int* OutVoxelCount);
        PL_RESULT JUCE_PUBLIC_FUNCTION GetVoxelLocation(PLVector* OutVoxelLocation, int Index);
        PL_RESULT JUCE_PUBLIC_FUNCTION GetVoxelAbsorpivity(float* OutAbsorpivity, int Index);
        PL_RESULT JUCE_PUBLIC_FUNCTION DrawGraph(PLVector GraphPosition);
        PL_RESULT JUCE_PUBLIC_FUNCTION Encode(PLVector EncodingPosition, int* OutVoxelIndex);
        PL_RESULT JUCE_PUBLIC_FUNCTION GetOcclusion(PLVector EmitterLocation, float* OutOcclusion);
    };
}
