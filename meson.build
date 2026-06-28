project(
  'yar',
  ['c', 'cpp'],
  default_options: [
    'default_library=shared',
    'c_std=c23',
    'cpp_std=c++23',
    'b_ndebug=if-release',
    'warning_level=3',
  ]
)

cpp = meson.get_compiler('cpp')
host_system = host_machine.system()
buildtype = get_option('buildtype')
source_dir = meson.current_source_dir()
cmake = import('cmake')

#ktx_opt = cmake.subproject_options()
#ktx_opt.add_cmake_defines({
#  'LIBKTX_VERSION_READ_ONLY': true,
#  'LIBKTX_VERSION_FULL': false,
#  'LIBKTX_FEATURE_KTX1': false,
#  'LIBKTX_FEATURE_KTX2': true,
#  'LIBKTX_FEATURE_VK_UPLOAD': true,
#  'LIBKTX_FEATURE_GL_UPLOAD': false,
#  'LIBKTX_FEATURE_ETC_UNPACK': false,
#})
#ktx_opt.set_override_option('cpp_std', 'c++23')
#ktx_proj = cmake.subproject('KTX-Software', options: ktx_opt)
#ktx_dep = ktx_proj.dependency('ktx')

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

includes = include_directories(
  'thirdparty/cgltf',

  'thirdparty/glm',

  'thirdparty/imgui',
  'thirdparty/imgui/backends',

  'thirdparty/stb',

  'thirdparty/VulkanMemoryAllocator/include',

  # Has to be first, otherwise will override our vulkan-headers.
  # is_system reverses include order.
  # https://mesonbuild.com/Reference-manual_functions_include_directories.html
  'thirdparty/KTX-Software/external/dfdutils',

  'thirdparty/vulkan-headers/include',

  'thirdparty/slang/build/slang-2026.3-linux-x86_64/include',

  'thirdparty/KTX-Software/lib/include',

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

slang_lib = cpp.find_library(
  'slang-compiler',
  dirs: [source_dir + '/thirdparty/slang/build/slang-2026.3-linux-x86_64/lib']
)

ktx_lib = cpp.find_library(
  'ktx',
  dirs: [source_dir + '/thirdparty/KTX-Software/lib/build']
)

subdir('src')
subdir('assets')

executable(
  'yar',
  yar_src + imgui_src,
  dependencies: [yar_deps, slang_lib, ktx_lib],
  include_directories: [includes],
)


