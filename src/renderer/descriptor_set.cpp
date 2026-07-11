#include <cstdint>
#include <cstring>
#include <memory>

#include "../public/material.h"
#include "../public/renderer/ibuffer.h"
#include "common.h"
#include "data_types.h"
#include "descriptor_set.h"
#include "renderer.h"

namespace yar
{
DescriptorSet::DescriptorSet(uint32_t maxFrames)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    m_vkDevice           = device.GetVkDevice();

    VkDescriptorSetLayoutBinding objectBinding = {};
    objectBinding.binding                      = BINDING_OBJECTS;
    objectBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    objectBinding.descriptorCount              = 1;
    objectBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    objectBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding vertexBinding = {};
    vertexBinding.binding                      = BINDING_VERTICES;
    vertexBinding.descriptorType               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexBinding.descriptorCount              = 1;
    vertexBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vertexBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding materialBinding = {};
    materialBinding.binding                      = BINDING_MATERIALS;
    materialBinding.descriptorType               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialBinding.descriptorCount              = 1;
    materialBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    materialBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding paramsBinding = {};
    paramsBinding.binding                      = BINDING_PARAMS;
    paramsBinding.descriptorType               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    paramsBinding.descriptorCount              = 1;
    paramsBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    paramsBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding textureBinding = {};
    textureBinding.binding                      = BINDING_TEXTURES;
    textureBinding.descriptorCount              = MAX_OBJECTS;
    textureBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBinding.pImmutableSamplers           = nullptr;

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
        objectBinding,
        vertexBinding,
        materialBinding,
        paramsBinding,
        textureBinding,
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
            vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_vkLayouts[i]),
            "Failed to create descriptor set layout"
        );
    }
}

DescriptorSet::~DescriptorSet()
{
    m_objectBuffers.clear();

    for (auto& layout : m_vkLayouts)
    {
        vkDestroyDescriptorSetLayout(m_vkDevice, layout, nullptr);
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
        vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_vkSets.data()),
        "Failed to allocate descriptor sets"
    );

    std::vector<VkWriteDescriptorSet>   writes  = {};
    std::vector<VkDescriptorBufferInfo> buffers = {};

    for (uint32_t i = 0; i < setCount; i++)
    {
        m_objectBuffers.push_back(
            std::make_shared<Buffer>(
                m_vkDevice,
                UniformBuffer,
                SecretThirdOption,
                sizeof(ShaderObjectData),
                MAX_OBJECTS
            )
        );

        VkDescriptorBufferInfo buffer = {};
        buffer.buffer                 = m_objectBuffers[i]->GetVkBuffer();
        buffer.offset                 = 0;
        buffer.range                  = sizeof(ShaderObjectData);
        buffers.push_back(buffer);

        VkWriteDescriptorSet write = {};
        write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet               = m_vkSets[i];
        write.dstBinding           = BINDING_OBJECTS;
        write.dstArrayElement      = 0;
        write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.descriptorCount      = 1;
        writes.push_back(write);

        m_vertexBuffers.push_back(
            std::make_shared<
                Buffer>(m_vkDevice, StorageBuffer, SecretThirdOption, sizeof(float), VERT_BUFF_SIZE)
        );

        buffer.buffer = m_vertexBuffers[i]->GetVkBuffer();
        buffer.offset = 0;
        buffer.range  = sizeof(float) * VERT_BUFF_SIZE;
        buffers.push_back(buffer);

        write.dstBinding     = BINDING_VERTICES;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes.push_back(write);

        m_materialBuffers.push_back(
            std::make_shared<Buffer>(
                m_vkDevice,
                StorageBuffer,
                SecretThirdOption,
                sizeof(ShaderMaterialData),
                MAX_OBJECTS
            )
        );

        buffer.buffer = m_materialBuffers[i]->GetVkBuffer();
        buffer.offset = 0;
        buffer.range  = sizeof(ShaderMaterialData) * MAX_OBJECTS;
        buffers.push_back(buffer);

        write.dstBinding = BINDING_MATERIALS;
        writes.push_back(write);

        m_parameterBuffers.push_back(
            std::make_shared<Buffer>(
                m_vkDevice,
                StorageBuffer,
                SecretThirdOption,
                sizeof(float),
                MAX_OBJECTS * 6
            )
        );

        buffer.buffer = m_parameterBuffers[i]->GetVkBuffer();
        buffer.offset = 0;
        buffer.range  = sizeof(float) * MAX_OBJECTS * 6;
        buffers.push_back(buffer);

        write.dstBinding = BINDING_PARAMS;
        writes.push_back(write);
    }

    for (uint32_t i = 0; i < writes.size(); i++)
    {
        writes[i].pBufferInfo = &buffers[i];
    }

    vkUpdateDescriptorSets(
        m_vkDevice,
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

void DescriptorSet::Update(uint32_t frameIndex, const std::vector<std::shared_ptr<Node>>& nodes)
{
    if (nodes.size() <= 0)
    {
        return;
    }

    if (nodes.size() >= MAX_OBJECTS)
    {
        throw std::runtime_error("exceeded max object count");
    }

    std::vector<VkDescriptorImageInfo> textureInfos = {};

    uint32_t firstTexIdx   = 0;
    uint32_t firstParamIdx = 0;
    uint32_t vertexOffset  = 0;

    for (uint32_t i = 0; i < nodes.size(); i++)
    {
        const auto& mat         = nodes[i]->GetMaterial();
        const auto  materialIdx = i;

        std::vector<ResourceHandle<Texture>> textures = {};

        for (const auto& tex : mat.GetTextures())
        {
            if (!tex.IsValid())
            {
                continue;
            }
            VkDescriptorImageInfo info = {};
            info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView             = tex->GetImageView();
            info.sampler               = tex->GetSampler();
            textureInfos.push_back(info);
            textures.push_back(tex);
        }

        const auto& params = mat.GetParameters();

        ShaderMaterialData matData = {};
        matData.params[0]          = textures.size() > 0 ? firstTexIdx : UINT32_MAX;
        matData.params[1]          = static_cast<uint32_t>(textures.size());
        matData.params[2]          = params.size() > 0 ? firstParamIdx : UINT32_MAX;
        matData.params[3]          = static_cast<uint32_t>(params.size());

        const auto&      trans  = nodes[i]->GetGlobalTransform();
        ShaderObjectData object = {};
        object.model            = trans.GetModelMatrix();
        object.normal           = trans.GetRotationMatrix();
        object.params[0]        = materialIdx;

        m_parameterBuffers[frameIndex]
            ->Write(params.data(), params.size() * sizeof(float), firstParamIdx * sizeof(float));
        m_materialBuffers[frameIndex]
            ->Write(&matData, sizeof(ShaderMaterialData), materialIdx * sizeof(ShaderMaterialData));
        m_objectBuffers[frameIndex]
            ->Write(&object, sizeof(ShaderObjectData), i * sizeof(ShaderObjectData));

        const auto& vertices = nodes[i]->GetVertices();
        m_vertexBuffers[frameIndex]
            ->Write(vertices.data(), vertices.size() * sizeof(float), vertexOffset * sizeof(float));

        firstTexIdx += textures.size();
        firstParamIdx += params.size();
        vertexOffset += vertices.size();
    }

    VkWriteDescriptorSet textureWrite = {};
    textureWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    textureWrite.dstSet               = m_vkSets[frameIndex];
    textureWrite.dstBinding           = BINDING_TEXTURES;
    textureWrite.dstArrayElement      = 0;
    textureWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureWrite.descriptorCount      = static_cast<uint32_t>(textureInfos.size());
    textureWrite.pImageInfo           = textureInfos.data();

    std::array<VkWriteDescriptorSet, 1> writes = {textureWrite};

    vkUpdateDescriptorSets(
        m_vkDevice,
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );

    auto& stats = g_renderer->GetRenderStats();
    stats.TextureCount += static_cast<uint32_t>(textureInfos.size());
}

void DescriptorSet::SetSky(const SkyComponent* sky)
{
    const auto                         color         = sky->GetColor();
    const auto                         lut           = sky->GetLUT();
    const auto                         diffuse       = sky->GetDiffuse();
    const auto                         specular      = sky->GetSpecular();
    const uint32_t                     setCount      = static_cast<uint32_t>(m_vkLayouts.size());
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
        colorInfo.imageView             = color->GetImageView();
        colorInfo.sampler               = color->GetSampler();
        colorInfos.push_back(colorInfo);

        VkDescriptorImageInfo lutInfo = {};
        lutInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        lutInfo.imageView             = lut->GetImageView();
        lutInfo.sampler               = lut->GetSampler();
        lutInfos.push_back(lutInfo);

        VkDescriptorImageInfo diffuseInfo = {};
        diffuseInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        diffuseInfo.imageView             = diffuse->GetImageView();
        diffuseInfo.sampler               = diffuse->GetSampler();
        diffuseInfos.push_back(diffuseInfo);

        VkDescriptorImageInfo specularInfo = {};
        specularInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        specularInfo.imageView             = specular->GetImageView();
        specularInfo.sampler               = specular->GetSampler();
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
        m_vkDevice,
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
