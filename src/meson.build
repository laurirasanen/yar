sdl_dep = dependency('sdl3', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

#slang_dep = declare_dependency(
#  link_args: ['-lslang-compiler'],
#  include_directories: include_directories(
#    '../thirdparty/slang/build/slang-2026.3-linux-x86_64/include/'
#  )
#)
#
#ktx_dep = declare_dependency(
#  link_args: ['-lktx'],
#  include_directories: include_directories('../thirdparty/KTX-Software/lib/include/')
#)

yar_src = files([
  'components/scene.cpp',
  'components/texture.cpp',

  'platform/fs.cpp',

  'renderer/buffer.cpp',
  'renderer/descriptor_set.cpp',
  'renderer/device.cpp',
  'renderer/image.cpp',
  'renderer/instance.cpp',
  'renderer/renderer.cpp',
  'renderer/vma.cpp',

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
]

