#!/bin/bash

set -euo pipefail

rm -rf ./build

meson setup build -Dbuildtype=debug -Doptimization=0 -Db_sanitize=address,undefined
meson compile -C build

