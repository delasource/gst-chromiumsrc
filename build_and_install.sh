#!/bin/bash
cmake -B build -DCMAKE_BUILD_TYPE=Release || exit
cmake --build build || exit
cmake --install build
