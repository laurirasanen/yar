#pragma once

#include <vulkan/vulkan_core.h>

#include "../shader/compiler.h"
#include "resource.h"

#include <format>
#include <string>

namespace yar
{
constexpr static VkShaderModuleCreateInfo GetShaderCreateInfo(const void* data, size_t size)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = static_cast<const uint32_t*>(data);
    return createInfo;
}

constexpr static VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
    VkShaderModuleCreateInfo* module,
    VkShaderStageFlagBits     stage
)
{
    VkPipelineShaderStageCreateInfo createInfo {};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = VK_NULL_HANDLE;
    createInfo.pName  = "main";
    createInfo.pNext  = module;
    return createInfo;
}

class Shader : public Resource
{
  public:
    explicit Shader(const std::string& id, const std::string& entry) :
        Resource(std::format("{}_{}", id, entry)),
        m_name(id),
        m_entry(entry)
    {
    }

    const VkShaderModuleCreateInfo& GetModuleInfo()
    {
        return m_moduleInfo;
    }

    const VkPipelineShaderStageCreateInfo& GetStageInfo()
    {
        return m_stageInfo;
    }

  protected:
    bool DoLoad() override
    {
        size_t      size;
        const void* spirv = g_shaderCompiler->GetSpirv(m_name, m_entry, size);
        if (!spirv)
        {
            throw std::runtime_error(
                std::format("failed to load shader {} entry {}", m_name, m_entry)
            );
        }

        m_moduleInfo = GetShaderCreateInfo(spirv, size);
        m_stageInfo  = FillShaderStageCreateInfo(&m_moduleInfo, VK_SHADER_STAGE_FRAGMENT_BIT);

        return true;
    }

    bool DoUnload() override
    {
        return true;
    }

  private:
    std::string                     m_name;
    std::string                     m_entry;
    VkShaderModuleCreateInfo        m_moduleInfo;
    VkPipelineShaderStageCreateInfo m_stageInfo;
};
}; // namespace yar
