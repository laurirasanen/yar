# Yet Another Renderer

A renderer written in modern C++ and Vulkan.

- C++23
- Vulkan 1.4
- SDL3
- Dear ImGui overlay
- Slang shader compile at runtime
- glTF scenes
- Frustum culling
- PBR (ORM workflow)
- Image based lighting

![Example scene](docs/cobble.gif)

# Building

## Linux

Dependencies:
- meson
- gcc
- sdl3
- vulkan

```sh
./build_deps.sh
./process_ibl.sh
meson setup build
meson compile -C build
```

## Acknowledgments

### Papers

- [Brent Burley. Physically Based Shading at Disney. 2012.](https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf)

### Other resources

- [Sascha Willems: Vulkan examples](https://github.com/SaschaWillems/Vulkan) (MIT)
- [Google: Filament](https://github.com/google/filament) (Apache 2.0)
- [Krzysztof Narkowicz: ACES Filmic Tone Mapping Curve](https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/) (CC0 / MIT)

### Third-party depedencies
- [cgltf](https://github.com/jkuhlmann/cgltf) (MIT)
- [glm](https://github.com/g-truc/glm) (MIT)
- [glTF-IBL-Sampler](https://github.com/KhronosGroup/glTF-IBL-Sampler) (Apache 2.0)
- [imgui](https://github.com/ocornut/imgui) (MIT)
- [KTX-Software](https://github.com/KhronosGroup/KTX-Software) (Apache 2.0)
- [SDL](https://github.com/libsdl-org/SDL) (zlib)
- [slang](https://github.com/shader-slang/slang) (Apache 2.0)
- [stb](https://github.com/nothings/stb) (MIT / Public Domain)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) (MIT)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) (Apache 2.0 / MIT)

### Assets

**assets/ibl/cobble/color.hdr**
- https://polyhaven.com/a/cobblestone_parish_road
- Elvis Posa, CC0

**assets/ibl/cloudy/color.hdr**
- https://polyhaven.com/a/overcast_soil_puresky
- Sergej Majboroda, Jarod Guest, CC0

**assets/scenes/DamagedHelmet.glb**
- A fictional helmet with textured damage.
- https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/DamagedHelmet
- (c) 2018 ctxwing, CC BY 4.0 International
  - ctxwing for Rebuild and conversion to glTF
- (c) 2016 theblueturtle_, CC BY-NC 4.0 International
  - theblueturtle_ for Earlier version of model

**assets/scenes/FlightHelmet.glb**
- Displayed flight helmet on a wooden stand.
- https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/FlightHelmet
- (c) 2018 Public, CC0 1.0 Universal
  - Gary Hsu for Conversion from Maya

