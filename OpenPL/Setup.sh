#!/bin/bash
#
# Builds and Downloads any and all external content
#

# Colours
NOCOLOR='\033[0m'
RED='\033[0;31m'
BLUE='\033[0;34m'
BOLD='\033[1m'

# Move into directory we were invoked in
cd "`dirname "$0"`"

echo -e "${RED}==> ${NOCOLOR}${BOLD}Installing OpenPL${NOCOLOR}"

# Stop advice for detatched heads
# We're not cloning the projects to make commits so this is fine
git config --global advice.detachedHead false

echo -e "${RED}==>${NOCOLOR} Installing JUCE"

# If JUCE isn't downloaded, clone
if [ ! -d "External/JUCE/" ]
then

    # Clone into External/
    cd External/
    mkdir JUCE
    # Need 2.3.0 because future versions of igl are having major revisions
    git clone https://github.com/juce-framework/JUCE.git --branch 6.0.7 --depth 1 JUCE
    
    cd ../
    
else
    echo -e "JUCE is already installed"
fi

echo -e "${RED}==>${NOCOLOR} Installing lbigl"

# If libigl isn't downloaded, clone
if [ ! -d "External/libigl/" ]
then

    # Clone into External/
    cd External/
    mkdir libigl
    # Need 2.3.0 because future versions of igl are having major revisions
    git clone https://github.com/libigl/libigl.git --branch v2.3.0 --depth 1 libigl
    
    cd libigl
    mkdir external
    cd external

    echo -e "${RED}==>${NOCOLOR} Installing lbigl dependencies"
    mkdir eigen
    mkdir glad
    mkdir glfw

    echo -e "${BLUE}==>${NOCOLOR} Installing eigen"
    git clone https://github.com/libigl/eigen.git --depth 1 eigen
    
    echo -e "${BLUE}==>${NOCOLOR} Installing glad"
    git clone https://github.com/libigl/libigl-glad.git --depth 1 glad
    
    echo -e "${BLUE}==>${NOCOLOR} Installing glfw"
    git clone https://github.com/glfw/glfw.git --depth 1 glfw

    # Go back up a folder for the following commands to work
    cd ../../../
    
else
    echo -e "libigl is already installed"
fi

if [ "$(uname)" = "Darwin" ]; then
    echo -e "${RED}==>${NOCOLOR} Installing macOS packages"
    echo -e "${RED}==>${NOCOLOR} Checking brew exists"

    which -s brew
    if [[ $? != 0 ]] ; then

        echo
        echo -e "${BLUE}Brew is not installed. Without it, you will have to manually install all library dependancies.${NOCOLOR}"
        echo
        
        read -p "Do you want to install brew? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]
            then
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            else
                echo -e "${RED}==>${NOCOLOR} Exiting. Please check the projucer file for all libraries required"
                exit 1
        fi
    fi
    
    brew install glfw

    brew install gmp

    brew install mpfr

    brew install boost
    
    brew install cgal

    brew install matplotplusplus
    
else
    echo -e "${RED}==>${NOCOLOR} Installing Linux packages"
    
    # JUCE Dependencies
    # https://github.com/juce-framework/JUCE/blob/develop/docs/Linux%20Dependencies.md
    
    sudo apt-get -y install g++
    
    sudo apt-get -y install libjack-jackd2-dev
    
    sudo apt-get -y install libext-dev
    
    sudo apt-get -y install libxrender-dev
    
    sudo apt-get -y install libwebkit2gtk-4.0-dev
    
    sudo apt-get -y install libglu1-mesa-dev
    
    sudo apt-get -y install mesa-common-dev
    
    sudo apt-get -y install libx11-dev
    
    sudo apt-get -y install libxinerama-dev
    
    sudo apt-get -y install libxrandr-dev
    
    sudo apt-get -y install libxcursor-dev
    
    sudo apt-get -y install mesa-common-dev
    
    sudo apt-get -y install libasound2-dev
    
    sudo apt-get -y install freeglut3-dev
    
    sudo apt-get -y install libxcomposite-dev
    
    sudo apt-get -y install libfreetype-dev libfreetype6 libfreetype6-dev
    
    sudo apt-get -y install curl
    
    sudo apt-get -y install libcurl4-openssl-dev
    
    # OpenPL Dependencies
        
    sudo apt-get -y install libglfw3-dev
    
    sudo apt-get -y install libgmp-dev
    
    sudo apt-get -y install libboost-all-dev
    
    sudo apt-get -y install lubmpfr-dev
    
    sudo apt-get -y install lubmpfr-dev
    
    sudo apt-get -y install libcgal-dev
    
    sudo apt-get -y install gnuplot
    
    sudo apt-get -y install cmake
    
    # On Linux, it's a bit of a pain to get matplotplusplus installed
    # Because of this, it's easier to clone the repo and update the header search paths
    
    echo -e "${RED}==>${NOCOLOR} Installing matplotplusplus and compiling from source"
    
    if [ ! -d "External/matplotplusplus/" ] ; then
    
    cd External
    mkdir matplotplusplus
    git clone https://github.com/alandefreitas/matplotplusplus.git --branch v1.0.1 --depth 1 matplotplusplus
    
    cd matplotplusplus
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_CXX_FLAGS="-O2" -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF
    sudo cmake --build . --parallel 2 --config Release
    sudo cmake --install .
    
    else
    	echo -e "matplotplusplus already installed"
    fi

fi

echo -e "${RED}==>${NOCOLOR} Done!"
