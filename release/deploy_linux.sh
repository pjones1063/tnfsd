#!/bin/bash
APP_NAME="TnfsTrayApp"
BUILD_DIR="../build_linux"

# Build the project
mkdir -p "$BUILD_DIR"
cmake -S .. -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j$(nproc)

# Create the .deb package
cd "$BUILD_DIR"
if cpack -G DEB; then
    mv *.deb "../release/"
fi

# Create the Portable .tar.gz
cd "../release"
tar -cvzf ${APP_NAME}-linux-portable.tar.gz -C "$BUILD_DIR" ${APP_NAME}

# Clean up
rm -rf "$BUILD_DIR"
echo "Done! Deliverables are in release/ folder."
