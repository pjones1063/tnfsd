#!/bin/bash
APP_NAME="TnfsTrayApp"
BUILD_DIR="../build_mac"

echo "==> Building $APP_NAME for macOS..."
cmake -S .. -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR --config Release -j$(sysctl -n hw.ncpu)

echo "==> Packaging DMG..."
macdeployqt $BUILD_DIR/${APP_NAME}.app -dmg

# Move deliverable to root and cleanup
mv $BUILD_DIR/${APP_NAME}.dmg .
rm -rf $BUILD_DIR

echo "Done! DMG is in release/."
