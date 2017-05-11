#!/bin/bash
# Sets up the development environment

# exit if any program fails
set -e

force=0;
if [ "$1" == "-f" ]; then
    force=1;
elif [ "$1" == 'clean' ]; then
    rm -rf JoystickLibrary
    rm Robot/lib*.a
    exit 0
fi

echo -e "\033[1;92m==== *nix Build Script for LAME ====\033[0m"

# verify repo up to date
git pull

# get dependencies
if [ -d "JoystickLibrary" ] && [ $force -eq 0 ]; then
    echo -e "\033[1;93mJoystickLibrary already exists, will not override. Use -f to override.\033[0m"
    cd JoystickLibrary/ && git pull && cd ..
else
    rm -rf JoystickLibrary/
    git clone https://github.com/WisconsinRobotics/JoystickLibrary
fi

echo -e "\033[1;92m==== Building Dependencies ====\033[0m"
echo -e "\033[1;93mNote that the GUI cannot be built on *nix.\033[0m"

# build dependencies
# JoystickLibrary 
pushd .
cd JoystickLibrary/cpp
rm -rf build/ && mkdir build && cd build
cmake .. && make
cp src/libJoystickLibrary.a  ../../../Robot
popd

echo -e "\033[1;92m==== All dependencies built successfully! ===== \033[0m"
