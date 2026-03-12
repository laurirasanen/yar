#include <cstdint>
#include <cstring>

#include "../DataTypes.h"
#include "VulkanCommon.h"
#include "VulkanDescriptorSet.h"

namespace drive
{
VulkanDescriptorSet::VulkanDescriptorSet(
    const VulkanDevice&                        device,
    std::vector<std::shared_ptr<VulkanBuffer>> uboBuffers
) :
    m_device(device),
    m_uniformBuffers(uboBuffers)
{
    VkDescriptorSetLayoutBinding uboBinding {};
    uboBinding.binding            = 0;
    uboBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount    = 1;
    uboBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &uboBinding;

    const auto maxFrames = static_cast<uint32_t>(m_uniformBuffers.size());
    m_vkLayouts.resize(maxFrames);

    for (uint32_t i = 0; i < maxFrames; i++)
    {
        VK_CHECK(
            vkCreateDescriptorSetLayout(
                device.GetVkDevice(),
                &layoutInfo,
                nullptr,
                &m_vkLayouts[i]
            ),
            "Failed to create descriptor set layout"
        );
    }

    m_vkSets.resize(maxFrames);

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = device.GetUboDescriptorPool();
    allocInfo.descriptorSetCount = maxFrames;
    allocInfo.pSetLayouts        = m_vkLayouts.data();

    VK_CHECK(
        vkAllocateDescriptorSets(device.GetVkDevice(), &allocInfo, m_vkSets.data()),
        "Failed to allocate descriptor sets"
    );

    m_ubosMappedMemory.resize(maxFrames);
    for (uint32_t i = 0; i < maxFrames; i++)
    {
        auto uniformBuffer = uboBuffers[i];
        uniformBuffer->Map(&m_ubosMappedMemory[i]);

        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = uniformBuffer->GetVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = m_vkSets[i];
        descriptorWrite.dstBinding      = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo     = &bufferInfo;

        vkUpdateDescriptorSets(m_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    m_uniformBuffers.clear();

    for (auto& layout : m_vkLayouts)
    {
        vkDestroyDescriptorSetLayout(m_device.GetVkDevice(), layout, nullptr);
    }
}

void VulkanDescriptorSet::UpdateUBO(uint32_t frameIndex, const UniformBufferObject* ubo)
{
    std::memcpy(m_ubosMappedMemory[frameIndex], ubo, sizeof(UniformBufferObject));
}

void VulkanDescriptorSet::Bind(
    VkCommandBuffer     commandBuffer,
    VkPipelineBindPoint bindPoint,
    VkPipelineLayout    pipelineLayout,
    uint32_t            frameIndex
)
{
    vkCmdBindDescriptorSets(
        commandBuffer,
        bindPoint,
        pipelineLayout,
        0,
        1,
        &m_vkSets[frameIndex],
        0,
        nullptr
    );
}
} // namespace drive
