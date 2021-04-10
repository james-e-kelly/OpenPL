/*
  ==============================================================================

    PL_SCENE.h
    Created: 3 Feb 2021 2:19:54pm
    Author:  James Kelly

  ==============================================================================
*/

#pragma once

#include "OpenPLCommon.h"
#include "OpenPLCommonPrivate.h"
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <atomic>

class Simulator;

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
    
    ~PL_SCENE();
    
    /**
     * Get the owning scene object.
     */
    PL_RESULT GetSystem(PL_SYSTEM** OutSystem) const;
    
    /**
     * Set the position of this scene. Will not create new voxels, but will update voxel position correctly if voxels are already created.
     *
     * @param ScenePosition Position of the scene
     */
    PL_RESULT SetScenePosition(const PLVector& ScenePosition);
    
    /**
     * Create the voxels for this scene.
     *
     * Use SetScenePosition to move the scene away from the default 0,0,0.
     * @param SceneSize Size of the voxel bounds
     * @param VoxelSize Size of each voxel
     */
    PL_RESULT CreateVoxels(const PLVector& SceneSize, float VoxelSize);
    
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
     * Converts the scene's geometry to voxels, ready for simulation.
     *
     * @param CenterPosition Center of the AABB containing all the voxels.
     * @param Size Size of the voxel lattice.
     * @param VoxelSize Size of each voxel cell.
     */
    PL_RESULT Voxelise(PLVector CenterPosition, PLVector Size, float VoxelSize = 5.f);
    
    PL_RESULT Simulate();
    
    PL_RESULT GetVoxelsCount(int* OutVoxelCount);
    
    PL_RESULT GetVoxelLocation(PLVector* OutVoxelLocation, int Index);
    
    PL_RESULT GetVoxelAbsorpivity(float* OutAbsorpivity, int Index);
    
    /**
     * Get all the meshes in this scene.
     */
    PL_RESULT GetMeshes(const std::vector<PL_MESH>** OutMeshes) const;
    
    PL_RESULT GetScenePosition(PLVector* OutScenePosition) const;
    
    PL_RESULT GetSceneSize(PLVector* OutSceneSize) const;
    
    PL_RESULT GetSceneVoxelSize(float* OutVoxelSize) const;
    
    PL_RESULT GetScenePositionBottomBackLeftCorner(PLVector* OutBottomBackLeftCorner) const;
    
    PL_RESULT GetVoxelPosition(int VoxelIndex, PLVector* OutVoxelLocation) const;
    
    PL_RESULT GetVoxelPosition(const PLVoxel& Voxel, PLVector* OutVoxelLocation) const;
    
    PL_RESULT GetVoxelAtPosition(const PLVector& Position, PLVoxel* OutVoxel) const;
    
private:
    PL_SYSTEM* OwningSystem;
    
    PLVector ScenePosition;
    PLVector SceneSize;
    float VoxelSize;
    
    std::vector<PL_MESH> Meshes;
    std::vector<PLVector> ListenerLocations;
    std::vector<PLVector> SourceLocations;
    
    PL_VOXEL_GRID Voxels;
    
    int TimeSteps = 100;
    
    /**Pointer to the object that will run our specific simulation. Can be anything from FDTD to rectangular decomposition etc.*/
    std::unique_ptr<Simulator> SimulatorPointer;
    
    enum ThreadStatus
    {
        ThreadStatus_NotStarted,
        ThreadStatus_Ongoing,
        ThreadStatus_Finished
    };
    
    boost::scoped_thread<> VoxelThread;
    boost::mutex VoxelMutex;
    std::atomic<ThreadStatus> VoxelThreadStatus { ThreadStatus_NotStarted };
    
    PL_RESULT VoxeliseInternal(PLVector CenterPosition, PLVector Size, float VoxelSize);
    
    /**
     * Adds absorption values to the voxel lattice cells based on the absorptivity of each mesh.
     *
     * If Voxelise creates the voxels, this method gives them meaning.
     */
    PL_RESULT FillVoxels();
};
