#!/bin/bash

set -euo pipefail

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/garden/color.hdr \
    -outCubeMap assets/ibl/garden/diffuse.ktx2 \
    -distribution Lambertian

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/garden/color.hdr \
    -outCubeMap assets/ibl/garden/specular.ktx2 \
    -distribution GGX

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/garden/color.hdr \
    -outLUT assets/ibl/garden/lut.png
