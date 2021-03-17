# OpenPL

OpenPL is an open source project created for a dissertation project at The University of Portsmouth.

The library connects to games engines like Unreal and Unity to provide wave simulation tools.

## Installation

This library requires [JUCE](https://juce.com) and [git](https://git-scm.com).

### Windows

Currently, there is no installation support for Windows.

### Mac

External libraries can be downloaded by running `Setup.sh`.

### Linux

On Ubuntu, and other similar distros, you can run `Setup.sh` to download all dependences.

`Setup.sh` uses `apt-get` to download the dependencies. If your distro has apt, you can run `Setup.sh`.

For other distros, checking `Setup.sh` will give you the list of packages required.

#### Arch Linux

If you are on Arch, you can install matplotplusplus with `yay -S matplotplusplus`.

For other dependencies, check `Setup.sh` for the packages required.

#### Matplotplusplus

If you cannot install a binary version of matplotplusplus, you must build and install it from source. `Setup.sh` shows an example of building the tool from source. Running this same code should install it for your system too.

## Running

For all platforms, you can open the `.jucer` file and select your desired exporter. The Projucer will open the project in your selected IDE.

## Building

### Windows

There is currently no build support for Windows.

### Mac

Open the `.jucer` file, open the project in Xcode and hit `Build` or `Run`.

### Linux

For Linux, `make` is required.

```
cd Builds
cd LinuxMakeFile
make
```


