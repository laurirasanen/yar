#!/bin/bash

set -euo pipefail

MESA_VK_TRACE_TRIGGER="/tmp/trigger" MESA_VK_TRACE="rgp" ./build/yar
