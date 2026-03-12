#!/bin/bash

set -euo pipefail

rm -rf ./build

meson setup -Dbuildtype=release -Doptimization=2 build
meson compile -C build
