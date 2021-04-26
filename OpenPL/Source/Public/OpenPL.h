/*
  ==============================================================================

    OpenPL.h
    Created: 15 Aug 2020 4:00:46pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * Sets the function OpenPL will call to output debug messages.
     *
     * Should be called as early as possible. Preferably before calling PL_System_Create as it allows any debug messages from that method to be displayed.
     *
     * In the example of Unity, you could create a method that takes the message and calls Debug.Log to output the message.
     *
     * @param Callback Pointer to a function with the PL_Debug_Callback signature.
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Debug_Initialize (PL_Debug_Callback Callback);

    /**
     * Creates a system object.
     * System objects are management objects for the simulation.
     * Only one is needed per game instance.
     *
     * @param OutSystem The created system object.
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_System_Create (PL_SYSTEM** OutSystem);

    /**
     * Destorys a system object and releases its resources.
     *
     * @param System System to destroy.
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_System_Release (PL_SYSTEM* System);
    
    /**
     * Set the main runtime listener position.
     *
     * @param System System to the set listener position for.
     * @param ListenerPosition Position of the new listener position
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_System_SetListenerPosition(PL_SYSTEM* System, PLVector ListenerPosition);

    /**
     * Get the listener position for the entire system.
     *
     * @param System System object to get the listener position from.
     * @param OutListenerPosition Listener position from the system.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_System_GetListenerPosition(PL_SYSTEM* System, PLVector* OutListenerPosition);

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
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_System_CreateScene(PL_SYSTEM* System, PL_SCENE** OutScene);

    /**
     * Releases and destroys a scene object.
     *
     * @param Scene Scene to destroy.
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_Release(PL_SCENE* Scene);

    /**
     * Splits up the simulation scene into voxels, ready to be filled by geometry and simulated over.
     *
     * @param Scene Scene to create voxels for.
     * @param SceneSize Size of the scene to create.
     * @param VoxelSize Size of each voxel (width and height of the cube).
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_CreateVoxels(PL_SCENE* Scene, PLVector SceneSize, float VoxelSize);
    
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
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_AddMesh(PL_SCENE* Scene, PLVector* WorldPosition, PLQuaternion* WorldRotation, PLVector* WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex);
    
    /**
     * Removes a mesh from the scene.
     * Uses the index passed from PL_Scene_AddMesh.
     *
     * @param Scene Scene to remove the mesh from.
     * @param IndexToRemove Index of the mesh to remove.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_RemoveMesh(PL_SCENE* Scene, int IndexToRemove);
    
    /**
     * Takes all the geometry in the scene and fills the voxels with the correct values.
     *
     * @param Scene Scene to fill geometry for.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_FillVoxelsWithGeometry(PL_SCENE* Scene);
    
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
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_AddListenerLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex);
    
    /**
     * Removes a listener from the scene.
     *
     * @param Scene Scene to remove the listener from.
     * @param IndexToRemove Listener index to remove.
     * @see PL_Scene_AddListenerLocation
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_RemoveListenerLocation(PL_SCENE* Scene, int IndexToRemove);
    
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
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_AddSourceLocation(PL_SCENE* Scene, PLVector* Position, int* OutIndex);
    
    /**
     * Removes a source from the scene.
     *
     * @param Scene Scene to remove the source from.
     * @param IndexToRemove Source index to remove.
     * @see PL_Scene_AddLSourceLocation
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_RemoveSourceLocation(PL_SCENE* Scene, int IndexToRemove);

    /**
     * Using the previously passed geometry, emitter locations and listener locations, simulate over the scene and store the resulting simulation data to disk.
     *
     * @param Scene Scene to run the simulation over.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_Simulate(PL_SCENE* Scene, PLVector SimulationLocation);
    
    /**
     * Opens a new OpenGL window and displays the meshes contained within the scene.
     * WARNING: Rendering must happen on the main thread, therefore the game will pause while the debug window is open.
     * WARNING: Context doesn't properly return to the game. You'll likely need to force quit the engine when exiting.
     * This method won't be included in release versions for this reason. The method is purely for debugging during development.
     *
     * @param Scene Scene to render.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_Debug(PL_SCENE* Scene);
    
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_GetVoxelsCount(PL_SCENE* Scene, int* OutVoxelCount);
    
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_GetVoxelLocation(PL_SCENE* Scene, PLVector* OutVoxelLocation, int Index);
    
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_GetVoxelAbsorpivity(PL_SCENE* Scene, float* OutAbsorpivity, int Index);

    /**
     * Render a graph with MatPlot++ at a location in the scene.
     *
     * @param Scene Scene to draw the graph of.
     * @param GraphPosition Position to draw the graph at.
    */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_DrawGraph(PL_SCENE* Scene, PLVector GraphPosition);

    /**
     * Create an impulse response from a set position.
     *
     * @param Scene Scene to build the impulse response from.
     * @param EncodingPosition Position of the impulse response.
     */
    PL_RESULT JUCE_PUBLIC_FUNCTION PL_Scene_Encode(PL_SCENE* Scene, PLVector EncodingPosition);
    
#ifdef __cplusplus
}
#endif
