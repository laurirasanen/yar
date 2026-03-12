sdl_dep = dependency('sdl2', include_type: 'system')
vk_dep = dependency('vulkan', include_type: 'system')

shadercompiler_src = files([
  'ShaderCompiler.cpp',
])

drive_src = files([
  'Renderer/Vulkan/VmaUsage.cpp',
  'Renderer/Vulkan/VulkanBuffer.cpp',
  'Renderer/Vulkan/VulkanDescriptorSet.cpp',
  'Renderer/Vulkan/VulkanDevice.cpp',
  'Renderer/Vulkan/VulkanInstance.cpp',
  'Renderer/Vulkan/VulkanRenderer.cpp',
  
  'UI/UI.cpp',

  'Window/Window.cpp',
  
  'World/Terrain.cpp',
  'World/World.cpp',

  'Engine.cpp',
  'main.cpp',
])

drive_deps = [
    sdl_dep,
    vk_dep,
]

