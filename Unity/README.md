# OpenPL Unity Project

This contains a demo Unity project demonstrating OpenPL.

Current development is geared towards the FMOD integration, however Wwise support is planned. When added, the user can switch between the two integrations.

## Installation

- Download [Unity Hub](https://unity3d.com/get-unity/download)
- Download Unity 2020.3.2f1(LTS) through Unity Hub
- Open the `OpenPL` Unity project

## Running and Building

All editing, running, playing, building and packing can run through Unity.

The OpenPL library files are included in the repository so don't need to be downloaded.

## Architecture

While not complete, the rough outline of code is as follows:

- Simulation: Uses the OpenPL library files with editor code to allow the user to run wave simulations from the editor
- Runtime: Takes data from the saved wave simulations and passing this to either FMOD or Wwise