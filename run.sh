#!/bin/bash

cd "${0%/*}"
export LD_LIBRARY_PATH="$(pwd)"
./sample
