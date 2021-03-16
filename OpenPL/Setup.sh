#!/bin/bash
#
# Builds and Downloads any and all external content
#

# Colours
NOCOLOR='\033[0m'
RED='\033[0;31m'
BLUE='\033[0;34m'

# Move into directory we were invoked in
cd "`dirname "$0"`"

echo "${RED}Generating OpenPL${NOCOLOR}"
echo

# Stop advice for detatched heads
# We're not cloning the projects to make commits so this is fine
git config --global advice.detachedHead false

# If libigl isn't downloaded, clone
if [ ! -d "External/libigl/" ]
then
    echo "${RED}Downloading libigl${NOCOLOR}"
    echo

    # Clone into External/
    cd External/
    mkdir libigl
    # Need 2.3.0 because future versions of igl are having major revisions
    git clone https://github.com/libigl/libigl.git --branch v2.3.0 --depth 1 libigl
    
    cd libigl
    mkdir external
    cd external

    echo "${RED}Downloading libigl dependancies${NOCOLOR}"
    echo
    mkdir eigen
    mkdir glad
    mkdir glfw

    echo "${BLUE}Downloading eigen${NOCOLOR}"
    git clone https://github.com/libigl/eigen.git --depth 1 eigen
    
    echo "${BLUE}Downloading glad${NOCOLOR}"
    git clone https://github.com/libigl/libigl-glad.git --depth 1 glad
    
    echo "${BLUE}Downloading glfw${NOCOLOR}"
    git clone https://github.com/glfw/glfw.git --depth 1 glfw

    # Go back up a folder for the following commands to work
    cd ../../../
    
else
    echo "${RED}libigl is installed${NOCOLOR}"
    echo
fi

if [ "$(uname)" = "Darwin" ]; then
    echo "${RED}Installing packages for macOS${NOCOLOR}"
    echo "${RED}Checking brew package manager${NOCOLOR}"
    echo

    which -s brew
    if [[ $? != 0 ]] ; then

        echo "${BLUE}Brew is not installed. Without it, you will have to manually install all library dependancies.${NOCOLOR}"
        echo
        
        read -p "Do you want to install brew? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]
            then
                echo "${RED}Installing brew${NOCOLOR}"
                echo
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            else
                echo "${RED}Exiting. Please check the projucer file for all libraries required${NOCOLOR}"
                echo
                exit 1
        fi
    fi
    
    brew install glfw

    brew install gmp

    brew install mpfr

    brew install boost

    brew install matplotplusplus
    
else
    echo "${RED}Installing packages for Linux${NOCOLOR}"

fi

echo
echo "${RED}Done!${NOCOLOR}"
