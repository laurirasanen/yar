#include <cstdint>
#include <cstring>
#include <memory>

#include "common.h"
#include "data_types.h"
#include "descriptor_set.h"
#include "image.h"
#include "material.h"
#include "renderer.h"

namespace yar
{
DescriptorSet::DescriptorSet(uint32_t maxFrames)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

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

    VkDescriptorSetLayoutBinding ormBinding = {};
    ormBinding.binding                      = BINDING_ORM;
    ormBinding.descriptorCount              = MAX_OBJECTS;
    ormBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ormBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    ormBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding normalBinding = {};
    normalBinding.binding                      = BINDING_NORMAL;
    normalBinding.descriptorCount              = MAX_OBJECTS;
    normalBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding emissiveBinding = {};
    emissiveBinding.binding                      = BINDING_EMISSIVE;
    emissiveBinding.descriptorCount              = MAX_OBJECTS;
    emissiveBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    emissiveBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissiveBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding iblColorBinding = {};
    iblColorBinding.binding                      = BINDING_IBL;
    iblColorBinding.descriptorCount              = 2;
    iblColorBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    iblColorBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    iblColorBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding iblLightingBinding = {};
    iblLightingBinding.binding                      = BINDING_IBL_FILTERED;
    iblLightingBinding.descriptorCount              = 2;
    iblLightingBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    iblLightingBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    iblLightingBinding.pImmutableSamplers           = nullptr;

    const std::array<VkDescriptorSetLayoutBinding, 7> bindings = {
        uboBinding,
        albedoBinding,
        ormBinding,
        normalBinding,
        emissiveBinding,
        iblColorBinding,
        iblLightingBinding
    };

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

DescriptorSet::~DescriptorSet()
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    m_objectBuffers.clear();

    for (auto& layout : m_vkLayouts)
    {
        vkDestroyDescriptorSetLayout(device.GetVkDevice(), layout, nullptr);
    }
}

void DescriptorSet::Alloc()
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    const uint32_t setCount = static_cast<uint32_t>(m_vkLayouts.size());
    m_vkSets.resize(setCount);

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = device.GetDescriptorPool();
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts        = m_vkLayouts.data();

    VK_CHECK(
        vkAllocateDescriptorSets(device.GetVkDevice(), &allocInfo, m_vkSets.data()),
        "Failed to allocate descriptor sets"
    );

    std::vector<VkWriteDescriptorSet>   writes      = {};
    std::vector<VkDescriptorBufferInfo> bufferInfos = {};
    writes.resize(setCount);
    bufferInfos.resize(setCount);

    for (uint32_t i = 0; i < setCount; i++)
    {
        m_objectBuffers.push_back(
            std::make_shared<Buffer>(
                device.GetVkDevice(),
                UniformBuffer,
                SecretThirdOption,
                sizeof(ShaderObjectData),
                MAX_OBJECTS
            )
        );

        bufferInfos[i].buffer = m_objectBuffers[i]->GetVkBuffer();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range  = sizeof(ShaderObjectData);

        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = m_vkSets[i];
        writes[i].dstBinding      = BINDING_UBO;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo     = &bufferInfos[i];
    }

    vkUpdateDescriptorSets(
        device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

constexpr uint32_t FillImageInfo(
    std::vector<VkDescriptorImageInfo>& infos,
    const std::shared_ptr<Texture>      tex
)
{
    if (tex != nullptr)
    {
        const auto            image = static_pointer_cast<VulkanImage>(tex->GetImage());
        VkDescriptorImageInfo info  = {};
        info.imageLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView              = image->GetVkImageView();
        info.sampler                = image->GetVkSampler();
        infos.push_back(info);
        return static_cast<uint32_t>(infos.size()) - 1;
    }

    return 0;
}

void DescriptorSet::Update(
    uint32_t                                         frameIndex,
    const std::vector<std::shared_ptr<IRenderNode>>& nodes
)
{
    if (nodes.size() <= 0)
    {
        return;
    }

    if (nodes.size() >= MAX_OBJECTS)
    {
        throw std::runtime_error("exceeded max object count");
    }

    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    std::vector<ShaderObjectData>      objects       = {};
    std::vector<VkDescriptorImageInfo> albedoInfos   = {};
    std::vector<VkDescriptorImageInfo> ormInfos      = {};
    std::vector<VkDescriptorImageInfo> normalInfos   = {};
    std::vector<VkDescriptorImageInfo> emissiveInfos = {};
    objects.resize(nodes.size());
    albedoInfos.reserve(nodes.size() + 1);
    ormInfos.reserve(nodes.size() + 1);
    normalInfos.reserve(nodes.size() + 1);
    emissiveInfos.reserve(nodes.size() + 1);

    FillImageInfo(albedoInfos, renderer->GetMissingTexture(TextureType::TEX_ALBEDO));
    FillImageInfo(ormInfos, renderer->GetMissingTexture(TextureType::TEX_ORM));
    FillImageInfo(normalInfos, renderer->GetMissingTexture(TextureType::TEX_NORMAL));
    FillImageInfo(emissiveInfos, renderer->GetMissingTexture(TextureType::TEX_EMISSIVE));

    for (size_t i = 0; i < nodes.size(); i++)
    {
        const auto mat      = static_pointer_cast<Material>(nodes[i]->GetMaterial());
        const auto albedo   = mat->GetAlbedo();
        const auto orm      = mat->GetORM();
        const auto normal   = mat->GetNormal();
        const auto emissive = mat->GetEmissive();

        objects[i].texIndices[OBJECT_TEX_ALBEDO]   = FillImageInfo(albedoInfos, albedo);
        objects[i].texIndices[OBJECT_TEX_ORM]      = FillImageInfo(ormInfos, orm);
        objects[i].texIndices[OBJECT_TEX_NORMAL]   = FillImageInfo(normalInfos, normal);
        objects[i].texIndices[OBJECT_TEX_EMISSIVE] = FillImageInfo(emissiveInfos, emissive);

        const auto lerp   = static_cast<float>(Time::TickFraction());
        const auto rt     = nodes[i]->GetInterpolatedTransform(lerp);
        objects[i].model  = rt.GetCombinedMatrix();
        objects[i].normal = rt.GetRotationMatrix();

        objects[i].materialParams.x = mat->GetOcclusionFactor();
        objects[i].materialParams.y = mat->GetRoughnessFactor();
        objects[i].materialParams.z = mat->GetMetalnessFactor();
        objects[i].materialParams.w = 0.0f;

        const float* emissiveFactor  = mat->GetEmissiveFactor();
        objects[i].materialParams2.x = emissiveFactor[0];
        objects[i].materialParams2.y = emissiveFactor[1];
        objects[i].materialParams2.z = emissiveFactor[2];
        objects[i].materialParams2.w = 0.0f;
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

    VkWriteDescriptorSet ormWrite = {};
    ormWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ormWrite.dstSet               = m_vkSets[frameIndex];
    ormWrite.dstBinding           = BINDING_ORM;
    ormWrite.dstArrayElement      = 0;
    ormWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ormWrite.descriptorCount      = static_cast<uint32_t>(ormInfos.size());
    ormWrite.pImageInfo           = ormInfos.data();

    VkWriteDescriptorSet normalWrite = {};
    normalWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    normalWrite.dstSet               = m_vkSets[frameIndex];
    normalWrite.dstBinding           = BINDING_NORMAL;
    normalWrite.dstArrayElement      = 0;
    normalWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalWrite.descriptorCount      = static_cast<uint32_t>(normalInfos.size());
    normalWrite.pImageInfo           = normalInfos.data();

    VkWriteDescriptorSet emissiveWrite = {};
    emissiveWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    emissiveWrite.dstSet               = m_vkSets[frameIndex];
    emissiveWrite.dstBinding           = BINDING_EMISSIVE;
    emissiveWrite.dstArrayElement      = 0;
    emissiveWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    emissiveWrite.descriptorCount      = static_cast<uint32_t>(emissiveInfos.size());
    emissiveWrite.pImageInfo           = emissiveInfos.data();

    std::array<VkWriteDescriptorSet, 4> writes =
        {albedoWrite, ormWrite, normalWrite, emissiveWrite};

    vkUpdateDescriptorSets(
        device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );

    auto& stats = g_renderer->GetRenderStats();
    stats.TextureCount += static_cast<uint32_t>(albedoInfos.size());
    stats.TextureCount += static_cast<uint32_t>(ormInfos.size());
    stats.TextureCount += static_cast<uint32_t>(normalInfos.size());
    stats.TextureCount += static_cast<uint32_t>(emissiveInfos.size());
}

void DescriptorSet::SetSky(std::shared_ptr<ISky> sky)
{
    auto        renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    const auto     color    = static_pointer_cast<VulkanImage>(sky->GetColor()->GetImage());
    const auto     lut      = static_pointer_cast<VulkanImage>(sky->GetLUT()->GetImage());
    const auto     diffuse  = static_pointer_cast<VulkanImage>(sky->GetDiffuse()->GetImage());
    const auto     specular = static_pointer_cast<VulkanImage>(sky->GetSpecular()->GetImage());
    const uint32_t setCount = static_cast<uint32_t>(m_vkLayouts.size());
    std::vector<VkWriteDescriptorSet>  writes        = {};
    std::vector<VkDescriptorImageInfo> colorInfos    = {};
    std::vector<VkDescriptorImageInfo> lutInfos      = {};
    std::vector<VkDescriptorImageInfo> diffuseInfos  = {};
    std::vector<VkDescriptorImageInfo> specularInfos = {};
    writes.reserve(setCount * 4);
    colorInfos.reserve(setCount);
    lutInfos.reserve(setCount);
    diffuseInfos.reserve(setCount);
    specularInfos.reserve(setCount);

    for (uint32_t i = 0; i < setCount; i++)
    {
        VkDescriptorImageInfo colorInfo = {};
        colorInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorInfo.imageView             = color->GetVkImageView();
        colorInfo.sampler               = color->GetVkSampler();
        colorInfos.push_back(colorInfo);

        VkDescriptorImageInfo lutInfo = {};
        lutInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        lutInfo.imageView             = lut->GetVkImageView();
        lutInfo.sampler               = lut->GetVkSampler();
        lutInfos.push_back(lutInfo);

        VkDescriptorImageInfo diffuseInfo = {};
        diffuseInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        diffuseInfo.imageView             = diffuse->GetVkImageView();
        diffuseInfo.sampler               = diffuse->GetVkSampler();
        diffuseInfos.push_back(diffuseInfo);

        VkDescriptorImageInfo specularInfo = {};
        specularInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        specularInfo.imageView             = specular->GetVkImageView();
        specularInfo.sampler               = specular->GetVkSampler();
        specularInfos.push_back(specularInfo);

        VkWriteDescriptorSet colorWrite = {};
        colorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        colorWrite.dstSet               = m_vkSets[i];
        colorWrite.dstBinding           = BINDING_IBL;
        colorWrite.dstArrayElement      = 0;
        colorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        colorWrite.descriptorCount      = 1;
        colorWrite.pImageInfo           = &colorInfos.back();
        writes.push_back(colorWrite);

        VkWriteDescriptorSet lutWrite = {};
        lutWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lutWrite.dstSet               = m_vkSets[i];
        lutWrite.dstBinding           = BINDING_IBL;
        lutWrite.dstArrayElement      = 1;
        lutWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        lutWrite.descriptorCount      = 1;
        lutWrite.pImageInfo           = &lutInfos.back();
        writes.push_back(lutWrite);

        VkWriteDescriptorSet diffuseWrite = {};
        diffuseWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        diffuseWrite.dstSet               = m_vkSets[i];
        diffuseWrite.dstBinding           = BINDING_IBL_FILTERED;
        diffuseWrite.dstArrayElement      = 0;
        diffuseWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        diffuseWrite.descriptorCount      = 1;
        diffuseWrite.pImageInfo           = &diffuseInfos.back();
        writes.push_back(diffuseWrite);

        VkWriteDescriptorSet specularWrite = {};
        specularWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        specularWrite.dstSet               = m_vkSets[i];
        specularWrite.dstBinding           = BINDING_IBL_FILTERED;
        specularWrite.dstArrayElement      = 1;
        specularWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        specularWrite.descriptorCount      = 1;
        specularWrite.pImageInfo           = &specularInfos.back();
        writes.push_back(specularWrite);
    }

    vkUpdateDescriptorSets(
        device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

void DescriptorSet::Bind(
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
