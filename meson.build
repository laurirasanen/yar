project(
  'drive',
  'cpp',
  default_options: [
    'default_library=static',
    'cpp_std=c++23',
    'b_ndebug=if-release',
    'warning_level=3',
  ]
)

cpp = meson.get_compiler('cpp')
compiler = cpp.get_id()
host_system = host_machine.system()
buildtype = get_option('buildtype')
source_dir = meson.current_source_dir()

# Sanity checks
if compiler not in ['gcc', 'msvc']
  error('Unsupported compiler ', compiler)
endif

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
endif

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(linker_args), language: 'cpp')

includes = include_directories(
  'include/VulkanMemoryAllocator/include',

  'include/glm',

  'include/imgui',
  'include/imgui/backends',

  'include/PerlinNoise',

  is_system: true,
)

subdir('src')

# TODO: is there a way to disable warnings for these?
imgui_src = [
  'include/imgui/imgui.cpp',
  'include/imgui/imgui_demo.cpp',
  'include/imgui/imgui_draw.cpp',
  'include/imgui/imgui_tables.cpp',
  'include/imgui/imgui_widgets.cpp',

  'include/imgui/backends/imgui_impl_sdl2.cpp',
  'include/imgui/backends/imgui_impl_vulkan.cpp',
]

shadercompiler = executable('shadercompiler', shadercompiler_src)
shader_headers = custom_target(
  'shaders',
  output: 'Shaders.h',
  command: [shadercompiler, source_dir + '/src/Shaders', source_dir + '/src/Generated'],
  build_always: true,
)

src = drive_src + imgui_src + shader_headers
deps = drive_deps
inc = includes

executable('drive', src,
  dependencies: deps,
  include_directories: inc
)


