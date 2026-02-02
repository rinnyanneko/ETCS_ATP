#!/bin/bash

echo "Building Taiwan Railway ATP System..."

# 檢查是否有必要的依賴
check_dependency() {
    if ! pkg-config --exists $1; then
        echo "Warning: $1 not found"
        return 1
    fi
    return 0
}

echo "Checking dependencies..."
DEPS_OK=true

if ! check_dependency sdl2; then
    echo "Please install SDL2 development libraries:"
    echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev"
    echo "  CentOS/RHEL: sudo yum install SDL2-devel"
    echo "  Arch: sudo pacman -S sdl2"
    DEPS_OK=false
fi

if ! check_dependency SDL2_ttf; then
    echo "Please install SDL2_ttf development libraries:"
    echo "  Ubuntu/Debian: sudo apt-get install libsdl2-ttf-dev"
    echo "  CentOS/RHEL: sudo yum install SDL2_ttf-devel"
    echo "  Arch: sudo pacman -S sdl2_ttf"
    DEPS_OK=false
fi

if [ "$DEPS_OK" = false ]; then
    echo "Please install missing dependencies and try again."
    exit 1
fi

echo "Creating build directory..."
mkdir -p build
cd build

echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DSIMRAIL=ON -DETCS_VENDORED=OFF

if [ $? -ne 0 ]; then
    echo "CMake configuration failed. Trying with vendored libraries disabled..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSIMRAIL=ON -DETCS_VENDORED=OFF -DRADIO_CFM=OFF
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration still failed. Please check dependencies."
    exit 1
fi

echo "Building project..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Taiwan Railway ATP system is ready."
    echo "Run ./DMI/dmi to start the system."
else
    echo "Build failed!"
    exit 1
fi