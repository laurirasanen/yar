#!/bin/bash

set -euo pipefail

cd build
meson compile
LSAN_OPTIONS="suppressions=../suppr.txt" ./drive

