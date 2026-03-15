#!/bin/bash

set -euo pipefail

cd build
meson compile
LD_LIBRARY_PATH="/usr/local/lib" LSAN_OPTIONS="suppressions=../suppr.txt" ./yar

