#!/bin/bash

set -euo pipefail

meson setup build -Dbuildtype=debug -Doptimization=0 -Db_sanitize=address,undefined

