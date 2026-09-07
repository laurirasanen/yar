#!/bin/bash

set -euo pipefail

STEAM="$HOME/.steam/steam"
RT="$STEAM/steamapps/common/SteamLinuxRuntime_4"

PRESSURE_VESSEL_OPTIONS=""
GAME_OPTIONS=""

cd /tmp/yar/

"$RT/run" \
  $PRESSURE_VESSEL_OPTIONS \
  -- \
  ./run.sh \
  $GAME_OPTIONS

