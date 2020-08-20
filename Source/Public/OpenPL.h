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
}
