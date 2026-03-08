#!/bin/bash
# Build DIV Games Studio (MSYS2 mingw32 + SDL2)
# Run from project root: ./build.sh [--clean]

set -e

export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"

if [ "$1" = "--clean" ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

mkdir -p build && cd build

cmake -G "MinGW Makefiles" \
    -DTARGETOS=WINDOWS-NATIVE \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Wno-dev \
    .. \
&& mingw32-make -j$(nproc)
