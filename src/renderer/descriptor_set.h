#pragma once

#include <vulkan/vulkan_core.h>

#include "../public/ecs/sky.h"
#include "buffer.h"
#include "node.h"

#include <memory>

namespace yar
{
enum DESC_BINDING : uint32_t
{
    BINDING_OBJECTS      = 0u,
    BINDING_VERTICES     = 1u,
    BINDING_MATERIALS    = 2u,
    BINDING_PARAMS       = 3u,
    BINDING_TEXTURES     = 4u,
    BINDING_IBL          = 5u,
    BINDING_IBL_FILTERED = 6u,
};

class DescriptorSet
{
  public:
    DescriptorSet() = delete;
    DescriptorSet(uint32_t maxFrames);
    ~DescriptorSet();

    DescriptorSet(const DescriptorSet&)            = delete;
    DescriptorSet(DescriptorSet&&)                 = default;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet& operator=(DescriptorSet&&)      = delete;

    void Alloc();

    void Update(uint32_t frameIndex, const std::vector<std::shared_ptr<Node>>& nodes);

    void SetSky(const SkyComponent* sky);

    void Bind(
        VkCommandBuffer     commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout    pipelineLayout,
        uint32_t            frameIndex,
        uint32_t            objectIndex
    );

    const std::vector<VkDescriptorSetLayout>& GetLayouts() const
    {
        return m_vkLayouts;
    }

  private:
    std::vector<VkDescriptorSetLayout>   m_vkLayouts;
    std::vector<VkDescriptorSet>         m_vkSets;
    std::vector<std::shared_ptr<Buffer>> m_objectBuffers;
    std::vector<std::shared_ptr<Buffer>> m_vertexBuffers;
    std::vector<std::shared_ptr<Buffer>> m_materialBuffers;
    std::vector<std::shared_ptr<Buffer>> m_parameterBuffers;
};
} // namespace yar
