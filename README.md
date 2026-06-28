# Yet Another Renderer

A renderer written in modern C++ and Vulkan.

- C++23
- Vulkan 1.4
- SDL3
- Dear ImGui overlay
- Slang shader compile at runtime
- GLTF scenes
- Frustum culling
- PBR (Metallic / Roughness / AO)

# Building

## Linux

Dependencies:
- meson
- gcc
- sdl3
- vulkan
- slang

See install_slang.sh if your distro doesn't provide a package for Slang.

```sh
meson setup build
meson compile -C build
```

## Acknowledgments

### Resources

- [Sascha Willems: Vulkan examples](https://github.com/SaschaWillems/Vulkan) (MIT)
- [Joey de Vries: Learn OpenGL - Theory](https://learnopengl.com/PBR/Theory) (CC BY-NC 4.0)

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

