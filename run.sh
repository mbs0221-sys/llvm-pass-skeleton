#!/bin/bash

export LLVM_HOME="/usr/lib/llvm-11"
cd build
rm CMakeCache.txt
cmake ..
make clean
make
cd ..
make clean
make example
spike pk example