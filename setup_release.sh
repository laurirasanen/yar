#!/bin/bash

set -euo pipefail

meson setup -Dbuildtype=release -Doptimization=2 build

