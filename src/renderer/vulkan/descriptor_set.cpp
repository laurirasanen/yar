#include <cstdint>
#include <cstring>
#include <vulkan/vulkan_core.h>

#include "../buffer.h"
#include "../data_types.h"
#include "common.h"
#include "descriptor_set.h"

namespace yar
{
VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDevice& device, uint32_t maxFrames) :
    m_device(device)
{
    VkDescriptorSetLayoutBinding uboBinding {};
    uboBinding.binding            = 0;
    uboBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboBinding.descriptorCount    = 1;
    uboBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &uboBinding;

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

    for (uint32_t i = 0; i < maxFrames; i++)
    {
        m_objectBuffers.push_back(
            std::make_shared<VulkanBuffer>(
                m_device.GetVkDevice(),
                UniformBuffer,
                SecretThirdOption,
                sizeof(ShaderObjectData),
                MAX_OBJECTS
            )
        );

        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = m_objectBuffers[i]->GetVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(ShaderObjectData);

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = m_vkSets[i];
        descriptorWrite.dstBinding      = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo     = &bufferInfo;

        vkUpdateDescriptorSets(m_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    m_objectBuffers.clear();

    for (auto& layout : m_vkLayouts)
    {
        vkDestroyDescriptorSetLayout(m_device.GetVkDevice(), layout, nullptr);
    }
}

uint32_t VulkanDescriptorSet::AppendShaderObjectData(
    uint32_t                frameIndex,
    const ShaderObjectData* data
)
{
    if (m_objectIndex >= MAX_OBJECTS)
    {
        throw std::runtime_error("exceeded max object count");
    }
    std::memcpy(
        static_cast<char*>(m_objectBuffers[frameIndex]->GetAllocationInfo().pMappedData)
            + m_objectIndex * sizeof(ShaderObjectData),
        data,
        sizeof(ShaderObjectData)
    );
    return m_objectIndex++;
}

void VulkanDescriptorSet::Bind(
    VkCommandBuffer     commandBuffer,
    VkPipelineBindPoint bindPoint,
    VkPipelineLayout    pipelineLayout,
    uint32_t            frameIndex,
    uint32_t            objectIndex
)
{
    uint32_t offsets[] = {objectIndex * static_cast<uint32_t>(sizeof(ShaderObjectData))};
    vkCmdBindDescriptorSets(
        commandBuffer,
        bindPoint,
        pipelineLayout,
        0,
        1,
        &m_vkSets[frameIndex],
        1,
        offsets
    );
}
} // namespace yar
