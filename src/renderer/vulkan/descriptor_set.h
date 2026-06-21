#pragma once

#include <vulkan/vulkan_core.h>

#include "../data_types.h"
#include "buffer.h"
#include "device.h"

namespace yar
{
#define MAX_OBJECTS 2048

class VulkanDescriptorSet
{
  public:
    VulkanDescriptorSet() = delete;
    VulkanDescriptorSet(const VulkanDevice& device, uint32_t maxFrames);
    ~VulkanDescriptorSet();

    VulkanDescriptorSet(const VulkanDescriptorSet&)            = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&&)                 = default;
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&&)      = delete;

    void NewFrame()
    {
        m_objectIndex = 0;
    }

    uint32_t AppendShaderObjectData(uint32_t frameIndex, const ShaderObjectData* data);

    void Bind(
        VkCommandBuffer     commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout    pipelineLayout,
        uint32_t            frameIndex,
        uint32_t            objectIndex
    );

    std::vector<VkDescriptorSetLayout> GetLayouts() const
    {
        return m_vkLayouts;
    }

  private:
    const VulkanDevice&                        m_device;
    std::vector<VkDescriptorSetLayout>         m_vkLayouts;
    std::vector<VkDescriptorSet>               m_vkSets;
    std::vector<std::shared_ptr<VulkanBuffer>> m_objectBuffers;
    uint32_t                                   m_objectIndex;
};
} // namespace yar
