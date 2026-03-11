#!/bin/bash
APP_NAME="TnfsTrayApp"
VERSION="1.0.0"
ARCH=$(dpkg --print-architecture)
BUILD_DIR="../build_pi"
PKG_NAME="${APP_NAME}_${VERSION}_${ARCH}"

echo "==> Building for Raspberry Pi..."
cmake -S .. -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR --config Release -j$(nproc)

# Create DEB structure
mkdir -p ${PKG_NAME}/usr/local/bin
mkdir -p ${PKG_NAME}/DEBIAN
cp $BUILD_DIR/$APP_NAME ${PKG_NAME}/usr/local/bin/

cat <<EOF > ${PKG_NAME}/DEBIAN/control
Package: tnfstrayapp
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: Paul
Description: FujiNet Protocol Server for Qt6
EOF

dpkg-deb --build ${PKG_NAME}
mv ${PKG_NAME}.deb .

# Cleanup
rm -rf ${PKG_NAME}
rm -rf $BUILD_DIR
echo "Done! ${PKG_NAME}.deb is in release/."
