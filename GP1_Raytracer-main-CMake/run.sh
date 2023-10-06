##!/bin/bash
#
#rm -rf build
#mkdir build && cd build
#cmake .. && cmake --build .
#cd ..
#build/game



#!/bin/bash

set -e

mkdir -p build; cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j`nproc`
cd .. && SDL_VIDEODRIVER=wayland build/game