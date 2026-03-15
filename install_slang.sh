#!/bin/bash

set -euo pipefail

cd ../slang

# needed for lib naming scheme
git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'

git checkout v2026.3
git submodule update --init --recursive

cmake --preset default
cmake --build --preset releaseWithDebugInfo
sudo cmake --build build --preset releaseWithDebugInfo --target install
