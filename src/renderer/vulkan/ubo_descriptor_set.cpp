#include <cstdint>
#include <cstring>
#include <vulkan/vulkan_core.h>

#include "../buffer.h"
#include "../data_types.h"
#include "common.h"
#include "image.h"
#include "ubo_descriptor_set.h"

namespace yar
{
UboDescriptorSet::UboDescriptorSet(const VulkanDevice& device, uint32_t maxFrames) :
    m_device(device)
{
    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding                      = BINDING_UBO;
    uboBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboBinding.descriptorCount              = 1;
    uboBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding albedoBinding = {};
    albedoBinding.binding                      = BINDING_ALBEDO;
    albedoBinding.descriptorCount              = MAX_OBJECTS;
    albedoBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding normalBinding = {};
    normalBinding.binding                      = BINDING_NORMAL;
    normalBinding.descriptorCount              = MAX_OBJECTS;
    normalBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding ormBinding = {};
    ormBinding.binding                      = BINDING_ORM;
    ormBinding.descriptorCount              = MAX_OBJECTS;
    ormBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ormBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    ormBinding.pImmutableSamplers           = nullptr;

    const std::array<VkDescriptorSetLayoutBinding, 4> bindings =
        {uboBinding, albedoBinding, normalBinding, ormBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

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
}

UboDescriptorSet::~UboDescriptorSet()
{
    m_objectBuffers.clear();

    for (auto& layout : m_vkLayouts)
    {
        vkDestroyDescriptorSetLayout(m_device.GetVkDevice(), layout, nullptr);
    }
}

void UboDescriptorSet::Alloc()
{
    const uint32_t setCount = static_cast<uint32_t>(m_vkLayouts.size());
    m_vkSets.resize(setCount);

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_device.GetUboDescriptorPool();
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts        = m_vkLayouts.data();

    VK_CHECK(
        vkAllocateDescriptorSets(m_device.GetVkDevice(), &allocInfo, m_vkSets.data()),
        "Failed to allocate descriptor sets"
    );

    for (uint32_t i = 0; i < setCount; i++)
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

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer                 = m_objectBuffers[i]->GetVkBuffer();
        bufferInfo.offset                 = 0;
        bufferInfo.range                  = sizeof(ShaderObjectData);

        VkWriteDescriptorSet bufferWrite = {};
        bufferWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bufferWrite.dstSet               = m_vkSets[i];
        bufferWrite.dstBinding           = BINDING_UBO;
        bufferWrite.dstArrayElement      = 0;
        bufferWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bufferWrite.descriptorCount      = 1;
        bufferWrite.pBufferInfo          = &bufferInfo;

        std::array<VkWriteDescriptorSet, 1> writes = {bufferWrite};

        vkUpdateDescriptorSets(
            m_device.GetVkDevice(),
            static_cast<uint32_t>(writes.size()),
            writes.data(),
            0,
            nullptr
        );
    }
}

void UboDescriptorSet::Update(
    uint32_t                                                frameIndex,
    const std::vector<std::shared_ptr<Mesh<VertexShaded>>>& meshes
)
{
    if (meshes.size() <= 0)
    {
        return;
    }

    if (meshes.size() >= MAX_OBJECTS)
    {
        throw std::runtime_error("exceeded max object count");
    }

    std::vector<ShaderObjectData>      objects     = {};
    std::vector<VkDescriptorImageInfo> albedoInfos = {};
    std::vector<VkDescriptorImageInfo> normalInfos = {};
    std::vector<VkDescriptorImageInfo> ormInfos    = {};
    objects.resize(meshes.size());
    albedoInfos.resize(meshes.size());
    normalInfos.resize(meshes.size());
    ormInfos.resize(meshes.size());

    for (size_t i = 0; i < meshes.size(); i++)
    {
        const auto mat    = meshes[i]->GetMaterial();
        const auto albedo = mat->GetAlbedo()->GetImage();
        const auto normal = mat->GetNormal()->GetImage();
        const auto orm    = mat->GetORM()->GetImage();

        objects[i].model            = meshes[i]->GetTransform()->GetCombinedMatrix();
        objects[i].normal           = meshes[i]->GetTransform()->GetRotationMatrix();
        objects[i].index            = static_cast<uint32_t>(i);
        objects[i].materialParams.x = mat->GetMetalnessFactor();
        objects[i].materialParams.y = mat->GetRoughnessFactor();
        objects[i].materialParams.z = mat->GetOcclusionFactor();
        objects[i].materialParams.w = 0.0f;

        albedoInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoInfos[i].imageView   = albedo->GetVkImageView();
        albedoInfos[i].sampler     = albedo->GetVkSampler();

        normalInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalInfos[i].imageView   = normal->GetVkImageView();
        normalInfos[i].sampler     = normal->GetVkSampler();

        ormInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ormInfos[i].imageView   = orm->GetVkImageView();
        ormInfos[i].sampler     = orm->GetVkSampler();
    }

    m_objectBuffers[frameIndex]->Write(objects.data(), objects.size() * sizeof(ShaderObjectData));

    VkWriteDescriptorSet albedoWrite = {};
    albedoWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    albedoWrite.dstSet               = m_vkSets[frameIndex];
    albedoWrite.dstBinding           = BINDING_ALBEDO;
    albedoWrite.dstArrayElement      = 0;
    albedoWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoWrite.descriptorCount      = static_cast<uint32_t>(albedoInfos.size());
    albedoWrite.pImageInfo           = albedoInfos.data();

    VkWriteDescriptorSet normalWrite = {};
    normalWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    normalWrite.dstSet               = m_vkSets[frameIndex];
    normalWrite.dstBinding           = BINDING_NORMAL;
    normalWrite.dstArrayElement      = 0;
    normalWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalWrite.descriptorCount      = static_cast<uint32_t>(normalInfos.size());
    normalWrite.pImageInfo           = normalInfos.data();

    VkWriteDescriptorSet ormWrite = {};
    ormWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ormWrite.dstSet               = m_vkSets[frameIndex];
    ormWrite.dstBinding           = BINDING_ORM;
    ormWrite.dstArrayElement      = 0;
    ormWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ormWrite.descriptorCount      = static_cast<uint32_t>(ormInfos.size());
    ormWrite.pImageInfo           = ormInfos.data();

    std::array<VkWriteDescriptorSet, 3> writes = {albedoWrite, normalWrite, ormWrite};

    vkUpdateDescriptorSets(
        m_device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

void UboDescriptorSet::Bind(
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
