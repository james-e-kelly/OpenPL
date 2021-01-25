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
 6. Pick out salient parameters
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
    /**
     * Sets the function OpenPL will call to output debug messages.
     *
     * Should be called as early as possible. Preferably before calling PL_System_Create as it allows any debug messages from that method to be displayed.
     *
     * In the example of Unity, you could create a method that takes the message and calls Debug.Log to output the message.
     *
     * @param Callback Pointer to a function with the PL_Debug_Callback signature.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Debug_Initialize (PL_Debug_Callback Callback);
    
    /**
     * Creates a system object.
     * System objects are management objects for the simulation.
     * Only one is needed per game instance.
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
     * Generally, only one scene object needs to be alive at once as (normally) there is only one scene open in the editor.
     *
     * PL_SCENE objects don't have a one-to-one relationship with game scenes. For example, sub-scenes within the game don't need their own PL_SCENE objects.
     * PL_SCENEs are just a way to collect geometry from the game and convert it all into one simulation scene.
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
    
    // TODO: AddMesh needs to take the absoption value of the mesh.
    // Can we get away with one absorption per mesh?
    
    /**
     * Adds a game mesh to the PL_SCENE.
     * Parameters like WorldPosition offset the mesh's vertices to reflect mesh's position in the game.
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
     * Takes all meshes within the scene and turns them into one voxel lattice, ready to simulate over.
     *
     * To help with performance, the size of the voxel lattice can be set. As there is a fixed size, some meshes/faces could be excluded.
     *
     * @param Scene Scene to voxelise.
     * @param CenterPosition World center position of the lattice.
     * @param Size Total size of the lattice.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_Voxelise(PL_SCENE* Scene, PLVector* CenterPosition, PLVector* Size);
    
    /**
     * Adds a listener location to the scene.
     *
     * Scenes must have at least one listener for simulations to work.
     *
     * @param Scene Scene to add the listener to.
     * @param Position Position of the listener.
     * @param OutIndex Index of the listener in the array.
     * @see PL_Scene_RemoveListenerLocation
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_AddListenerLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex);
    
    /**
     * Removes a listener from the scene.
     *
     * @param Scene Scene to remove the listener from.
     * @param IndexToRemove Listener index to remove.
     * @see PL_Scene_AddListenerLocation
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_RemoveListenerLocation(PL_SCENE* Scene, int IndexToRemove);
    
    /**
     * Adds a source location to the scene.
     * Source locations are audio emitter locations.
     *
     * Scenes must have at least one source for simulations to work.
     *
     * @param Scene Scene to add the source to.
     * @param Position Position of the source.
     * @param OutIndex Index of the source in the array.
     * @see PL_Scene_RemoveSourceLocation
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_AddSourceLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex);
    
    /**
     * Removes a source from the scene.
     *
     * @param Scene Scene to remove the source from.
     * @param IndexToRemove Source index to remove.
     * @see PL_Scene_AddLSourceLocation
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_RemoveSourceLocation(PL_SCENE* Scene, int IndexToRemove);
    
    /**
     * Opens a new OpenGL window and displays the meshes contained within the scene.
     * WARNING: Rendering must happen on the main thread, therefore the game will pause while the debug window is open.
     * WARNING: Context doesn't properly return to the game. You'll likely need to force quit the engine when exiting.
     * This method won't be included in release versions for this reason. The method is purely for debugging during development.
     *
     * @param Scene Scene to render.
     */
    JUCE_PUBLIC_FUNCTION PL_RESULT PL_Scene_Debug(PL_SCENE* Scene);
}
