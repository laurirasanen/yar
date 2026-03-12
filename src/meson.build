sdl_dep = dependency('sdl2', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

yar_src = files([
  'renderer/vulkan/vma_usage.cpp',
  'renderer/vulkan/vulkan_buffer.cpp',
  'renderer/vulkan/vulkan_descriptor_set.cpp',
  'renderer/vulkan/vulkan_device.cpp',
  'renderer/vulkan/vulkan_instance.cpp',
  'renderer/vulkan/vulkan_renderer.cpp',
  
  'ui/ui.cpp',

  'window/window.cpp',
  
  'world/world.cpp',

  'engine.cpp',
  'main.cpp',
])

yar_shaders = files([
  'shaders/fullscreen_fs.frag',
  'shaders/fullscreen_vs.vert',
  'shaders/simple_fs.frag',
  'shaders/simple_vs.vert',
  'shaders/sky_fs.frag',
  'shaders/sky_vs.vert',
])

yar_deps = [
    sdl_dep,
    vk_dep,
]

