#!/bin/bash

set -euo pipefail

rm -rf ./build

meson setup build -Dbuildtype=debug -Doptimization=0
meson compile -C build

