sdl_dep = dependency('sdl2', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

yar_src = files([
  'Renderer/Vulkan/VmaUsage.cpp',
  'Renderer/Vulkan/VulkanBuffer.cpp',
  'Renderer/Vulkan/VulkanDescriptorSet.cpp',
  'Renderer/Vulkan/VulkanDevice.cpp',
  'Renderer/Vulkan/VulkanInstance.cpp',
  'Renderer/Vulkan/VulkanRenderer.cpp',
  
  'UI/UI.cpp',

  'Window/Window.cpp',
  
  'World/World.cpp',

  'Engine.cpp',
  'main.cpp',
])

yar_shaders = files([
  'Shaders/fullscreen_fs.frag',
  'Shaders/fullscreen_vs.vert',
  'Shaders/simple_fs.frag',
  'Shaders/simple_vs.vert',
  'Shaders/sky_fs.frag',
  'Shaders/sky_vs.vert',
])

yar_deps = [
    sdl_dep,
    vk_dep,
]

