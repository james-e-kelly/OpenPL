/*
  ==============================================================================

    OpenPL.h
    Created: 15 Aug 2020 4:00:46pm
    Author:  James Kelly

  ==============================================================================
*/

/**
 Open Propagation Library

 Development Notes to Self:
 
 Editor Flow (Uses this library)
 -------------------
 
 1. Engine Editor Loads
 2. Load OpenPL
 3. Run Simulation over game scene(s)*
 4. Shutdown Editor
 5. Shutdown OpenPL
 
 Runtime Flow (Uses custom wrapper for Wwise/FMOD)
 -------------------
 At runtime, it's up to the engine to read data previously made by the simulation. OpenPL is not needed at this point.
 
 1. Load Game
 2. Load Scene
 3. Connect to Wwise/FMOD reverb bus plugin
 4. Load Simulation Data from Disk as needed
 5. Send simple simulation data as parameters to event/gameobject
 6. Send IRs to reverb bus plugin
 
 *Simulation Flow
 -------------------
 
 1. Pass in game geometry
 2. Voxelise the geometry
 3. Return the voxel geometry for scene debugging and visualisation
 4. Apply Rectangular Decomposition
 5. Run Simulation
 6. Pick out key parameters and IRs
 7. Save data to disk
 8. Return success
 
*/

#pragma once

#include <JuceHeader.h>
#include "OpenPLCommon.h"

typedef struct PL_SYSTEM    PL_SYSTEM;
typedef struct PL_SCENE     PL_SCENE;

extern "C"
{
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Debug_Initialize (PL_Debug_Callback Callback);
    
    /**
     * Creates a system object.
     * System objects are management objects for the simulation.
     *
     * @param OutSystem The created system object.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_System_Create (PL_SYSTEM** OutSystem);

    /**
     * Destorys a system object and releases its resources.
     *
     * @param System System to destroy.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_System_Release (PL_SYSTEM* System);

    /**
     * Creates a new scene object.
     * Scene objects can contain scene geometry, convert the geometry to voxels and act as an entry point for the simulation.
     *
     * @param System System object to create the scene.
     * @param OutScene Created scene.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_System_CreateScene(PL_SYSTEM* System, PL_SCENE** OutScene);

    /**
     * Releases and destroys a scene object.
     *
     * @param Scene Scene to destroy.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_Release(PL_SCENE* Scene);
    
    /**
     * Adds a mesh (vertices and indices) to the scene.
     * The function returns an index of where the mesh is stored inside the scene's internal mesh array so you can later remove that mesh.
     *
     * @param Scene Scene to add the mesh to.
     * @param Vertices Pointer to the start of a vertices array.
     * @param VerticesLength Length of the vertices array.
     * @param Indices Pointer to the start of an indices array.
     * @param IndicesLength Length of the indicies array.
     * @param OutIndex If successful, the index the mesh is stored at for later deletion.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_AddMesh(PL_SCENE* Scene, PLVector* WorldPosition, PLQuaternion* WorldRotation, PLVector* WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex);
    
    /**
     * Removes a mesh from the scene.
     * Uses the index passed from PL_Scene_AddMesh.
     *
     * @param Scene Scene to remove the mesh from.
     * @param IndexToRemove Index of the mesh to remove.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_RemoveMesh(PL_SCENE* Scene, int IndexToRemove);
    
    /**
     * Takes the meshes within the scene and turns them into a voxel lattice, ready to simulate over.
     *
     * To help with performance, the size of voxel lattice can be set. As there is a fixed size, some meshes/faces could be excluded.
     *
     * @param Scene Scene to voxelise.
     * @param CenterPosition World center position of the lattice.
     * @param Size Total size of the lattice.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_Voxelise(PL_SCENE* Scene, PLVector* CenterPosition, PLVector* Size);
    
    /**
     * Opens a new OpenGL window and displays the meshes contained within the scene.
     * WARNING: Rendering must happen on the main thread, therefore the game will pause while the debug window is open.
     *
     * @param Scene Scene to render.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_Debug(PL_SCENE* Scene);
}
