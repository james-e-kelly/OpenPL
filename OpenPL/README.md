This folder contains the main editor library which should be added to a game engine's editor to allow for wave simulation.

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
 5. Add source/emitter locations
 6. Add listener locations
 7. Run Simulation
 8. Pick out salient parameters
 9. Save data to disk
 10. Return success
 
