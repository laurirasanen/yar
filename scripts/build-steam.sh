#!/bin/bash

# Build script for steamrt.
#
# $ toolbox create -i registry.gitlab.steamos.cloud/steamrt/steamrt4/sdk steamrt4
# $ toolbox enter steamrt4
# $ sudo apt update && sudo apt install gcc-14-monolithic patchelf
# $ exit
#
# $ toolbox run -c steamrt4 ./scripts/build-sniper.sh
#

set -euo pipefail

rm -rf build-steam
rm -rf /tmp/yar

export CC=gcc-14
export CXX=g++-14

# deps
read -p "build deps? [y/N]" prompt
if [[ $prompt == "y" ]]; then
    ./scripts/build_deps.sh
fi

# build
meson setup build-steam -Dbuildtype=release -Doptimization=2 --prefix=/tmp/yar
meson compile -C build-steam

# dist
rm -rf /tmp/yar
meson install -C build-steam
find /tmp/yar/ -name 'meson.build' -delete
mv /tmp/yar/lib/* /tmp/yar/
rmdir /tmp/yar/lib
cp thirdparty/slang/build/slang-2026.3-linux-x86_64/lib/libslang-compiler.so /tmp/yar/
cp thirdparty/KTX-Software/lib/build/libktx.so /tmp/yar/
