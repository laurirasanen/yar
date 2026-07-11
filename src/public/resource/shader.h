#pragma once

#include <vulkan/vulkan_core.h>

#include "../shader/compiler.h"
#include "../util.h"
#include "resource.h"

#include <cstring>
#include <string>
#include <unordered_map>

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
    explicit Shader(const std::string& id) : Resource(id), m_name(id)
    {
    }

    bool HasEntry(const std::string& entry)
    {
        return m_moduleInfo.contains(entry);
    }

    const VkShaderModuleCreateInfo& GetModuleInfo(const std::string& entry)
    {
        return m_moduleInfo[entry];
    }

    const VkPipelineShaderStageCreateInfo& GetStageInfo(const std::string& entry)
    {
        return m_stageInfo[entry];
    }

  protected:
    bool DoLoad() override
    {
        const std::string entryNames[] =
            {SHADER_ENTRY_VERTEX, SHADER_ENTRY_PIXEL, SHADER_ENTRY_COMPUTE};
        const VkShaderStageFlagBits stages[] =
            {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};

        size_t size;
        for (uint32_t i = 0; i < ARRAY_SIZE(entryNames); i++)
        {
            const void* spirv = g_shaderCompiler->GetSpirv(m_name, entryNames[i], size);
            if (!spirv)
            {
                continue;
            }

            m_moduleInfo[entryNames[i]] = GetShaderCreateInfo(spirv, size);
            m_stageInfo[entryNames[i]] =
                FillShaderStageCreateInfo(&m_moduleInfo[entryNames[i]], stages[i]);
        }

        return true;
    }

    bool DoUnload() override
    {
        return true;
    }

  private:
    std::string                                                      m_name;
    std::unordered_map<std::string, VkShaderModuleCreateInfo>        m_moduleInfo;
    std::unordered_map<std::string, VkPipelineShaderStageCreateInfo> m_stageInfo;
};
}; // namespace yar
