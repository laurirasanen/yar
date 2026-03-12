#pragma once

#include <vulkan/vulkan_core.h>

#include <fullscreen_fs.h>
#include <fullscreen_vs.h>
#include <simple_fs.h>
#include <simple_vs.h>
#include <sky_fs.h>
#include <sky_vs.h>

namespace yar
{
class Shader
{
  public:
    Shader()                         = delete;
    ~Shader()                        = delete;
    Shader(const Shader&)            = delete;
    Shader(Shader&&)                 = delete;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&)      = delete;

    constexpr static VkShaderModuleCreateInfo GetVulkanCreateInfo(
        const void*  spv,
        const size_t size
    )
    {
        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode    = static_cast<const uint32_t*>(spv);
        return createInfo;
    }
};

// Usage:
// auto shader = LOAD_VULKAN_SPV(simple_fs);
#define LOAD_VULKAN_SPV(NAME) yar::Shader::GetVulkanCreateInfo(NAME, sizeof(NAME))
} // namespace yar
