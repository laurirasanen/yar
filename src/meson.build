sdl_dep = dependency('sdl3', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

slang_dep = declare_dependency(
  link_args: ['-lslang-compiler'],
  include_directories: include_directories('/usr/local/include')
)

yar_src = files([
  'platform/fs.cpp',

  'renderer/vulkan/vma.cpp',
  'renderer/vulkan/buffer.cpp',
  'renderer/vulkan/descriptor_set.cpp',
  'renderer/vulkan/device.cpp',
  'renderer/vulkan/instance.cpp',

  'renderer/renderer.cpp',

  'shader/compiler.cpp',

  'ui/ui.cpp',

  'window/window.cpp',

  'world/world.cpp',

  'engine.cpp',
  'main.cpp',
])

yar_deps = [
    sdl_dep,
    vk_dep,
    slang_dep
]

