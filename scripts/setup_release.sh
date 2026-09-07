#!/bin/bash

set -euo pipefail

rm -rf ./build

meson setup -Dbuildtype=release -Doptimization=2 --prefix=/tmp/yar build
meson compile -C build
