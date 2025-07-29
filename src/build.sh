#!/bin/bash
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm64.cmake ..
make -j$(nproc)