#!/bin/bash

set -euo pipefail

rm -rf /tmp/yar

meson install -C build

find /tmp/yar/ -name 'meson.build' -delete

mv /tmp/yar/lib/* /tmp/yar/
rmdir /tmp/yar/lib

cp thirdparty/slang/build/slang-2026.3-linux-x86_64/lib/libslang-compiler.so /tmp/yar/
cp thirdparty/KTX-Software/lib/build/libktx.so /tmp/yar/

cd /tmp
zip -r yar.zip yar/
