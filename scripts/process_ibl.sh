#!/bin/bash

set -euo pipefail

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cobble/color.hdr \
    -outCubeMap assets/ibl/cobble/diffuse.ktx2 \
    -distribution Lambertian

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cobble/color.hdr \
    -outCubeMap assets/ibl/cobble/specular.ktx2 \
    -distribution GGX

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cobble/color.hdr \
    -outLUT assets/ibl/cobble/lut.png

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outCubeMap assets/ibl/cloudy/diffuse.ktx2 \
    -distribution Lambertian

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outCubeMap assets/ibl/cloudy/specular.ktx2 \
    -distribution GGX

./thirdparty/glTF-IBL-Sampler/build/cli \
    -inputPath assets/ibl/cloudy/color.hdr \
    -outLUT assets/ibl/cloudy/lut.png
