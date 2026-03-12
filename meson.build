project(
  'yar',
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
if compiler not in ['gcc']
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
  'thirdparty/vulkan-headers/include',
  'thirdparty/VulkanMemoryAllocator/include',

  'thirdparty/glm',

  'thirdparty/imgui',
  'thirdparty/imgui/backends',

  is_system: true,
)

# TODO: is there a way to disable warnings for these?
imgui_src = [
  'thirdparty/imgui/imgui.cpp',
  'thirdparty/imgui/imgui_demo.cpp',
  'thirdparty/imgui/imgui_draw.cpp',
  'thirdparty/imgui/imgui_tables.cpp',
  'thirdparty/imgui/imgui_widgets.cpp',

  'thirdparty/imgui/backends/imgui_impl_sdl2.cpp',
  'thirdparty/imgui/backends/imgui_impl_vulkan.cpp',
]

glsl_compiler = find_program('glslang', 'glslangValidator')
glsl_args = [
  '--target-env', 'vulkan1.4',
  '--variable-name', '@BASENAME@',
  '--depfile', '@DEPFILE@',
  '@INPUT@',
  '-o', '@OUTPUT@',
]
glsl_generator = generator(
  glsl_compiler,
  output: ['@BASENAME@.h'],
  depfile: '@BASENAME@.h.d',
  arguments: glsl_args,
)

subdir('src')

executable(
  'yar',
  yar_src + imgui_src,
  glsl_generator.process(yar_shaders),
  dependencies: yar_deps,
  include_directories: includes
)


