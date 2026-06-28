#!/bin/bash
#
# Build CMake dependencies that Meson doesn't like.
#

set -euo pipefail

ROOT=$(pwd)

git submodule update --init --recursive

echo "BUILDING glTF-IBL-Sampler"
rm -rf "$ROOT/thirdparty/glTF-IBL-Sampler/build"
cd "$ROOT/thirdparty/glTF-IBL-Sampler"

# 2.8.12 in 2026? Khronos please
# support for < 3.5 has been dropped
export CMAKE_POLICY_VERSION_MINIMUM=3.5
cmake . -B build
cmake --build build

echo "BUILDING KTXLIB"
rm -rf "$ROOT/thirdparty/KTX-Software/lib/build"
cd "$ROOT/thirdparty/KTX-Software/lib"

cmake . -B build \
    -DLIBKTX_VERSION_READ_ONLY=OFF \
    -DLIBKTX_VERSION_FULL=ON \
    -DLIBKTX_FEATURE_KTX1=ON \
    -DLIBKTX_FEATURE_KTX2=ON \
    -DLIBKTX_FEATURE_VK_UPLOAD=ON \
    -DLIBKTX_FEATURE_GL_UPLOAD=OFF \
    -DLIBKTX_FEATURE_ETC_UNPACK=OFF

cmake --build build

echo "BUILDING SLANG"
rm -rf "$ROOT/thirdparty/slang/buid"
cd "$ROOT/thirdparty/slang"

# needed for lib naming scheme
git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'

cmake --preset default
cmake --build --preset releaseWithDebugInfo
