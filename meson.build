project(
  'yar',
  ['c', 'cpp'],
  default_options: [
    'default_library=shared',
    'c_std=c23',
    'cpp_std=c++23',
    'b_ndebug=if-release',
    'warning_level=3',
  ],
  meson_version: '>1.1',
)

cpp = meson.get_compiler('cpp')
host_system = host_machine.system()
buildtype = get_option('buildtype')
source_dir = meson.current_source_dir()
cmake = import('cmake')

compiler_args = [
  '-Wcast-align',
  '-Wconversion',
  '-Wdouble-promotion',
  '-Wduplicated-branches',
  '-Wduplicated-cond',
  '-Wformat=2',
  '-Wimplicit-fallthrough',
  '-Wlogical-op',
  '-Wmisleading-indentation',
  '-Wnon-virtual-dtor',
  '-Wnrvo',
  '-Wnull-dereference',
  '-Wold-style-cast',
  '-Woverloaded-virtual',
  '-Wshadow',
  '-Wsign-conversion',
  '-Wsuggest-override',
  '-Wunused',
  '-Wuseless-cast',

  '-DGLM_FORCE_DEPTH_ZERO_TO_ONE',

  '-m64',
]

linker_args = [
  '-m64',
]

# OS
if host_system == 'windows'
  compiler_args += [
    '-D_WINDOWS',
  ]
  compiler_args += [
    '-DWIN64',
    '-D_WIN64',
  ]
elif host_system == 'linux'
  compiler_args += [
    '-DLINUX',
    '-D_LINUX',
    '-DPOSIX',
  ]
  if buildtype == 'debug'
    linker_args += [
      '-rdynamic',
    ]
  endif
endif

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(linker_args), language: 'cpp')

thirdparty_inc = include_directories(
  # Has to be first, otherwise will override our vulkan-headers.
  # is_system reverses include order.
  # https://mesonbuild.com/Reference-manual_functions_include_directories.html
  'thirdparty/KTX-Software/external/dfdutils',

  'thirdparty/cgltf',

  'thirdparty/glm',

  'thirdparty/imgui',
  'thirdparty/imgui/backends',

  'thirdparty/stb',

  'thirdparty/VulkanMemoryAllocator/include',

  'thirdparty/slang/build/slang-2026.3-linux-x86_64/include',

  'thirdparty/KTX-Software/lib/include',

  'thirdparty/vulkan-headers/include',

  is_system: true,
)

# TODO: declare as dependency?
imgui_src = [
  'thirdparty/imgui/imgui.cpp',
  'thirdparty/imgui/imgui_demo.cpp',
  'thirdparty/imgui/imgui_draw.cpp',
  'thirdparty/imgui/imgui_tables.cpp',
  'thirdparty/imgui/imgui_widgets.cpp',

  'thirdparty/imgui/backends/imgui_impl_sdl3.cpp',
  'thirdparty/imgui/backends/imgui_impl_vulkan.cpp',
]

sdl_dep = dependency('sdl3', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

slang_lib = cpp.find_library(
  'slang-compiler',
  dirs: [source_dir + '/thirdparty/slang/build/slang-2026.3-linux-x86_64/lib']
)

ktx_lib = cpp.find_library(
  'ktx',
  dirs: [source_dir + '/thirdparty/KTX-Software/lib/build']
)

box3d_opt = cmake.subproject_options()
box3d_opt.add_cmake_defines({
  'BUILD_SHARED_LIBS': true,
  'BOX3D_SANITIZE': false,
  'BOX3D_DISABLE_SIMD': false,
  'BOX3D_COMPILE_WARNING_AS_ERROR': false,
  'BOX3D_DOUBLE_PRECISION': false,
  'BOX3D_SAMPLES': false,
  'BOX3D_BENCHMARKS': false,
  'BOX3D_DOCS': false,
  'BOX3D_PROFILE': false,
  'BOX3D_VALIDATE': false,
  'BOX3D_UNIT_TESTS': false,
  'BOX3D_BUILD_SHADERS': false,
})
box3d_proj = cmake.subproject('box3d', options: box3d_opt)
box3d_lib = box3d_proj.dependency('box3d')

engine_deps = [
    sdl_dep,
    vk_dep,
    slang_lib,
    ktx_lib,
    box3d_lib,
]

engine_src = files([
  'src/assets/gltf.cpp',

  'src/ecs/component.cpp',

  'src/engine/engine.cpp',
  'src/engine/entry.cpp',
  'src/engine/physics.cpp',
  'src/engine/time.cpp',

  'src/platform/fs.cpp',

  'src/renderer/buffer.cpp',
  'src/renderer/descriptor_set.cpp',
  'src/renderer/device.cpp',
  'src/renderer/instance.cpp',
  'src/renderer/post_process.cpp',
  'src/renderer/renderer.cpp',
  'src/renderer/scene.cpp',
  'src/renderer/vma.cpp',

  'src/resource/texture.cpp',

  'src/shader/compiler.cpp',

  'src/ui/ui.cpp',

  'src/window/window.cpp',

  'src/world/world.cpp',
])

engine_lib = library(
  'engine',
  engine_src + imgui_src,
  dependencies: [engine_deps],
  include_directories: [thirdparty_inc],
)

engine_dep = declare_dependency(
  link_with: [engine_lib],
  dependencies: [],
  include_directories: ['src/public', thirdparty_inc]
)

subdir('assets')

examples = get_option('examples')
if examples == true
  executable('sample', files(['examples/main.cpp']), dependencies: [engine_dep])
endif
