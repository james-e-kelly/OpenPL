# OpenPL (Open Propagation Library)

OpenPL is a [University of Portsmouth](https://www.port.ac.uk/) dissertation project to learn about [Project Triton/Acoustics](https://github.com/microsoft/ProjectAcoustics).

Written with [JUCE v6.0.1](https://github.com/juce-framework/JUCE).

## How To Install

### Mac

1) `cd DirectoryOfYourChoice`

2) `git clone https://github.com/KarateKidzz/OpenPL.git`

3) `git clone https://github.com/libigl/libigl.git`

4) Use libigl to download dependencies for us

```
cd ligigl
mkdir build
cd build
cmake ../
make

```

5) Download [JUCE 6.0.1](https://juce.com/get-juce/download) and place/copy in `DirectoryOfYourChoice`

6) Open the `.jucer` file in the Projucer app

### Windows

Currently, there are no exporters for Windows. However, adding an exporter to the `.jucer` is possible.

### Linux

Like Windows, there is no exporter for Linux set up but adding one is possible.