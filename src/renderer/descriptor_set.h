#pragma once

#include <vulkan/vulkan_core.h>

#include "../public/inode.h"
#include "../public/isky.h"
#include "../renderer/mesh.h"
#include "buffer.h"
#include "data_types.h"
#include "device.h"

#include <memory>

namespace yar
{
enum DESC_BINDING : uint32_t
{
    BINDING_UBO          = 0u,
    BINDING_ALBEDO       = 1u,
    BINDING_ORM          = 2u,
    BINDING_NORMAL       = 3u,
    BINDING_EMISSIVE     = 4u,
    BINDING_IBL          = 5u,
    BINDING_IBL_FILTERED = 6u,
};

class DescriptorSet
{
  public:
    DescriptorSet() = delete;
    DescriptorSet(const VulkanDevice& device, uint32_t maxFrames);
    ~DescriptorSet();

    DescriptorSet(const DescriptorSet&)            = delete;
    DescriptorSet(DescriptorSet&&)                 = default;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet& operator=(DescriptorSet&&)      = delete;

    void NewFrame()
    {
        m_objectIndex = 0;
    }

    void Alloc();

    void Update(uint32_t frameIndex, const std::vector<std::shared_ptr<INode>>& nodes);

    void SetSky(std::shared_ptr<ISky> sky);

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
    const VulkanDevice&                  m_device;
    std::vector<VkDescriptorSetLayout>   m_vkLayouts;
    std::vector<VkDescriptorSet>         m_vkSets;
    std::vector<std::shared_ptr<Buffer>> m_objectBuffers;
    uint32_t                             m_objectIndex;
};
} // namespace yar
