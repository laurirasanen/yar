#!/bin/bash

set -euo pipefail

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outCubeMap assets/ibl/cloudy/diffuse.ktx2 \
    -distribution Lambertian

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outCubeMap assets/ibl/cloudy/specular.ktx2 \
    -distribution GGX

../glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outLUT assets/ibl/cloudy/lut.png
