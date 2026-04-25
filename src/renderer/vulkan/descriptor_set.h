#pragma once

#include <vulkan/vulkan_core.h>

#include "../data_types.h"
#include "buffer.h"
#include "device.h"

namespace yar
{
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

    void Bind(
        VkCommandBuffer     commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout    pipelineLayout,
        uint32_t            frameIndex
    );

    std::vector<VkDescriptorSetLayout> GetLayouts() const
    {
        return m_vkLayouts;
    }

  private:
    const VulkanDevice&                m_device;
    std::vector<VkDescriptorSetLayout> m_vkLayouts;
    std::vector<VkDescriptorSet>       m_vkSets;
};
} // namespace yar
