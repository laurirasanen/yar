#include "post_process.h"
#include "../shader/compiler.h"
#include "device.h"
#include "renderer.h"
#include "src/renderer/common.h"
#include <vulkan/vulkan_core.h>

namespace yar
{
PostProcessPass::~PostProcessPass()
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    DestroyOutput();
    vkDestroyDescriptorSetLayout(device.GetVkDevice(), m_descriptorSetLayout, nullptr);
};

PostProcessPass::PostProcessPass(
    const char* name,
    const char* shader,
    uint32_t    numTextures,
    VkFormat    outputColorFormat
) :
    m_name(name),
    m_outputLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    size_t      size;

    const void* spirv = g_shaderCompiler->GetSpirv(shader, SHADER_ENTRY_PIXEL, size);
    if (!spirv)
    {
        throw std::runtime_error(std::format("failed to load {} fragment shader", shader));
    }

    auto fragModule = GetShaderCreateInfo(spirv, size);

    spirv = g_shaderCompiler->GetSpirv(shader, SHADER_ENTRY_VERTEX, size);
    if (!spirv)
    {
        throw std::runtime_error(std::format("failed to load {} vertex shader", shader));
    }

    auto        vertModule = GetShaderCreateInfo(spirv, size);
    auto        shaderFrag = FillShaderStageCreateInfo(&fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto        shaderVert = FillShaderStageCreateInfo(&vertModule, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector stages {shaderFrag, shaderVert};

    std::vector<VkDescriptorSetLayoutBinding> texBindings = {};
    texBindings.resize(numTextures);

    for (uint32_t i = 0; i < numTextures; i++)
    {
        texBindings[i].binding            = i;
        texBindings[i].descriptorCount    = 1;
        texBindings[i].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texBindings[i].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        texBindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(texBindings.size());
    layoutInfo.pBindings    = texBindings.data();

    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(
        vkCreateDescriptorSetLayout(
            device.GetVkDevice(),
            &layoutInfo,
            nullptr,
            &m_descriptorSetLayout
        ),
        "Failed to create descriptor set layout"
    );

    for (uint32_t i = 0; i < m_descriptorSets.size(); i++)
    {
        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = device.GetDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &m_descriptorSetLayout;

        LOG_DEBUG("ALLOC DESC SET");
        VK_CHECK(
            vkAllocateDescriptorSets(device.GetVkDevice(), &allocInfo, &m_descriptorSets[i]),
            "Failed to allocate descriptor set"
        );
    }

    const std::vector<VkDescriptorSetLayout> layouts = {m_descriptorSetLayout};

    m_pipeline = std::make_shared<VulkanPipeline<VertexEmpty>>(
        device.GetVkDevice(),
        stages,
        layouts,
        outputColorFormat,
        device.GetDepthFormat(),
        false,
        false
    );
}

void PostProcessPass::SetInputs(std::vector<RenderAttachment> inputs, uint32_t frameIndex)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    std::vector<VkDescriptorImageInfo> imageInfos = {};
    std::vector<VkWriteDescriptorSet>  writes     = {};
    imageInfos.resize(inputs.size());
    writes.resize(inputs.size());

    for (uint32_t i = 0; i < inputs.size(); i++)
    {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView   = inputs[i].ImageView;
        imageInfos[i].sampler     = inputs[i].Sampler;

        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = m_descriptorSets[frameIndex];
        writes[i].dstBinding      = i;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].descriptorCount = 1;
        writes[i].pImageInfo      = &imageInfos[i];
    }

    vkUpdateDescriptorSets(
        device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

void PostProcessPass::SetOutput(const RenderAttachment& output)
{
    DestroyOutput();
    m_colorOutput = output;
    m_ownOutput   = false;
}

void PostProcessPass::CreateOutput(uint32_t outputWidth, uint32_t outputHeight)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    const auto  vkDevice = device.GetVkDevice();

    CreateImage(
        &m_colorOutput.Image,
        &m_colorOutput.Allocation,
        VK_IMAGE_TYPE_2D,
        device.GetColorFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        outputWidth,
        outputHeight
    );
    renderer->SetDebugName(
        VK_OBJECT_TYPE_IMAGE,
        (uint64_t)m_colorOutput.Image,
        std::format("{} output color", m_name).c_str()
    );
    m_colorOutput.Width  = outputWidth;
    m_colorOutput.Height = outputHeight;

    CreateImageView(
        vkDevice,
        m_colorOutput.Image,
        &m_colorOutput.ImageView,
        VK_IMAGE_VIEW_TYPE_2D,
        device.GetColorFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT
    );
    renderer->SetDebugName(
        VK_OBJECT_TYPE_IMAGE_VIEW,
        (uint64_t)m_colorOutput.ImageView,
        std::format("{} output color view", m_name).c_str()
    );

    const float maxSamplerAnisotropy = device.GetProperties().limits.maxSamplerAnisotropy;
    CreateImageSampler(vkDevice, maxSamplerAnisotropy, &m_colorOutput.Sampler, m_outputSamplerMode);

    TransitionImageLayout(
        device.GetCommandBuffer(),
        m_colorOutput.Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    );

    m_ownOutput = true;
}

void PostProcessPass::Render(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    renderer->BeginDebugLabel(commandBuffer, m_name, {0.75f, 0.5f, 1.0f, 0.8f});

    VkClearValue clearColor {};
    clearColor.color = {
        {0.0f, 0.0f, 0.0f, 1.0f}
    };

    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.clearValue  = clearColor;
    colorAttachment.loadOp      = m_outputLoadOp;
    colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.imageView   = m_colorOutput.ImageView;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRect2D renderArea {};
    renderArea.extent = {.width = m_colorOutput.Width, .height = m_colorOutput.Height};
    renderArea.offset = {0, 0};

    VkRenderingInfo renderingInfo {};
    renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea           = renderArea;
    renderingInfo.viewMask             = 0;
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = nullptr;
    renderingInfo.pStencilAttachment   = nullptr;
    renderingInfo.flags                = 0;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    VkViewport viewport {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(m_colorOutput.Width);
    viewport.height   = static_cast<float>(m_colorOutput.Height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = {m_colorOutput.Width, m_colorOutput.Height};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(
        device.GetCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->GetVkPipelineLayout(),
        0,
        1,
        &m_descriptorSets[frameIndex],
        0,
        nullptr
    );

    vkCmdBindPipeline(
        device.GetCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->GetVkPipeline()
    );
    vkCmdPushConstants(
        device.GetCommandBuffer(),
        m_pipeline->GetVkPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(VkDeviceAddress),
        renderer->GetShaderGlobalBuffer()->GetDeviceAddress()
    );

    vkCmdDraw(device.GetCommandBuffer(), 3, 1, 0, 0);

    vkCmdEndRendering(commandBuffer);

    renderer->EndDebugLabel(commandBuffer);
}

void PostProcessPass::TransitionOutput()
{
    if (!m_ownOutput)
    {
        return;
    }

    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    TransitionImageLayout(
        device.GetCommandBuffer(),
        m_colorOutput.Image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_SHADER_READ_BIT
    );
}

void PostProcessPass::DestroyOutput()
{
    if (!m_ownOutput)
    {
        return;
    }

    const auto renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto vkDevice = renderer->GetDevice().GetVkDevice();

    vkDestroyImageView(vkDevice, m_colorOutput.ImageView, nullptr);
    vmaDestroyImage(g_vma, m_colorOutput.Image, m_colorOutput.Allocation);
}
}; // namespace yar
