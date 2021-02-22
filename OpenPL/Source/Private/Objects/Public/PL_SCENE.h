/*
  ==============================================================================

    PL_SCENE.h
    Created: 3 Feb 2021 2:19:54pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "../../../Public/OpenPLCommon.h"
#include "../../OpenPLCommonPrivate.h"
#include <vector>

/**
 * The scene class is the main work horse of the simulation.
 * 1) Take mesh data from game and convert to internal data types
 * 2) Convert to voxels
 * 3) Simulate over voxels
 * 4) Return simulated data/parameters
 * 5) Probably save to disk as well
 */
class PL_SCENE
{
public:
    
    /**
     * Constructor. Scene objects (and any other heap allocated objects) must reference the system object.
     * @param System System object that owns the scene.
     */
    PL_SCENE(PL_SYSTEM* System);
    
    /**
     * Get the owning scene object.
     */
    PL_RESULT GetSystem(PL_SYSTEM** OutSystem) const;
    
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
    PL_RESULT AddAndConvertGameMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex);
    
    /**
     * Add a PL_MESH to the scene's internal storage.
     *
     * @param Mesh Mesh to add.
     * @param OutIndex Index of the mesh within the array.
     */
    PL_RESULT AddMesh(PL_MESH& Mesh, int& OutIndex);
    
    /**
     * Removes a PL_MESH from the internal array using its index.
     *
     * @param Index Index to remove.
     */
    PL_RESULT RemoveMesh(int Index);
    
    /**
     * Add a listener to the simulation.
     *
     * @param Location Location of the new listener.
     * @param OutIndex Index of the new listener in the array.
     */
    PL_RESULT AddListenerLocation(PLVector& Location, int& OutIndex);
    
    /**
     * Remove a listener from the simulation.
     *
     * @param Index Index of the listener to remove. (Array so 0 =< Index < Size)
     */
    PL_RESULT RemoveListenerLocation(int Index);
    
    /**
     Add a source/emitter location to the simulation.
     *
     * @param Location Location of the new emitter.
     * @param OutIndex Index of the new emitter in the array.
     */
    PL_RESULT AddSourceLocation(PLVector& Location, int& OutIndex);
    
    /**
     * Remove an emitter from the simulation.
     *
     * @param Index Index of the emitter to remove. (Array so 0 =< Index < Size)
     */
    PL_RESULT RemoveSourceLocation(int Index);
    
    /**
     * Quick internal debugging method for displaying the meshes within the scene.
     */
    PL_RESULT OpenOpenGLDebugWindow() const;
    
    /**
     * Converts the scene's geometry to voxels, ready for simulation.
     *
     * @param CenterPosition Center of the AABB containing all the voxels.
     * @param Size Size of the voxel lattice.
     * @param VoxelSize Size of each voxel cell.
     */
    PL_RESULT Voxelise(PLVector CenterPosition, PLVector Size, float VoxelSize = 5.f);
    
    /**
     * Adds absorption values to the voxel lattice cells based on the absorptivity of each mesh.
     *
     * If Voxelise creates the voxels, this method gives them meaning.
     */
    PL_RESULT FillVoxels();
    
    PL_RESULT GetVoxelsCount(int* OutVoxelCount);
    
    PL_RESULT GetVoxelLocation(PLVector* OutVoxelLocation, int Index);
    
    PL_RESULT GetVoxelAbsorpivity(float* OutAbsorpivity, int Index);
    
private:
    PL_SYSTEM* OwningSystem;
    
    std::vector<PL_MESH> Meshes;
    std::vector<PLVector> ListenerLocations;
    std::vector<PLVector> SourceLocations;
    
    PL_VOXEL_GRID Voxels;
};
