#!/bin/bash

set -e

mkdir -p build; cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j`nproc`
cd .. && SDL_VIDEODRIVER=wayland build/game

