#include "post_process.h"
#include "../shader/compiler.h"
#include "device.h"
#include "renderer.h"

namespace yar
{
PostProcessPass::~PostProcessPass()
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    DestroyOutput();
    vkDestroyDescriptorSetLayout(device.GetVkDevice(), m_descriptorSetLayout, nullptr);
};

PostProcessPass::PostProcessPass(const char* shader, VkFormat outputColorFormat)
{
    const auto     renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto&    device   = renderer->GetDevice();
    ShaderCompiler compiler;
    size_t         size;

    const void* spirv = compiler.GetSpirv(shader, SHADER_ENTRY_PIXEL, size);
    if (!spirv)
    {
        throw std::runtime_error(std::format("failed to load {} fragment shader", shader));
    }

    auto fragModule = GetShaderCreateInfo(spirv, size);

    spirv = compiler.GetSpirv(shader, SHADER_ENTRY_VERTEX, size);
    if (!spirv)
    {
        throw std::runtime_error(std::format("failed to load {} vertex shader", shader));
    }

    auto        vertModule = GetShaderCreateInfo(spirv, size);
    auto        shaderFrag = FillShaderStageCreateInfo(&fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto        shaderVert = FillShaderStageCreateInfo(&vertModule, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector stages {shaderFrag, shaderVert};

    VkDescriptorSetLayoutBinding colorBinding = {};
    colorBinding.binding                      = 0;
    colorBinding.descriptorCount              = 1;
    colorBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    colorBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    colorBinding.pImmutableSamplers           = nullptr;

    VkDescriptorSetLayoutBinding depthBinding = {};
    depthBinding.binding                      = 1;
    depthBinding.descriptorCount              = 1;
    depthBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    depthBinding.pImmutableSamplers           = nullptr;

    const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {colorBinding, depthBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

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

void PostProcessPass::SetInputs(
    const RenderAttachment& colorInput,
    const RenderAttachment& depthInput,
    uint32_t                frameIndex
)
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    m_colorInput = colorInput;
    m_depthInput = depthInput;

    VkDescriptorImageInfo colorInfo = {};
    colorInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorInfo.imageView             = m_colorInput.ImageView;
    colorInfo.sampler               = m_colorInput.Sampler;

    VkWriteDescriptorSet colorWrite = {};
    colorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    colorWrite.dstSet               = m_descriptorSets[frameIndex];
    colorWrite.dstBinding           = 0;
    colorWrite.dstArrayElement      = 0;
    colorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    colorWrite.descriptorCount      = 1;
    colorWrite.pImageInfo           = &colorInfo;

    VkDescriptorImageInfo depthInfo = {};
    depthInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthInfo.imageView             = m_colorInput.ImageView;
    depthInfo.sampler               = m_colorInput.Sampler;

    VkWriteDescriptorSet depthWrite = {};
    depthWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    depthWrite.dstSet               = m_descriptorSets[frameIndex];
    depthWrite.dstBinding           = 1;
    depthWrite.dstArrayElement      = 0;
    depthWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthWrite.descriptorCount      = 1;
    depthWrite.pImageInfo           = &depthInfo;

    std::array<VkWriteDescriptorSet, 2> writes = {colorWrite, depthWrite};

    vkUpdateDescriptorSets(
        device.GetVkDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

void PostProcessPass::SetOutput(const VkImage output)
{
    DestroyOutput();
    m_colorOutput.Image = output;
    m_ownOutput         = false;
}

void PostProcessPass::CreateOutput()
{
    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();
    const auto  vkDevice = device.GetVkDevice();
    int         width;
    int         height;
    g_window->GetFramebufferSize(&width, &height);

    CreateImage(
        &m_colorOutput.Image,
        &m_colorOutput.Allocation,
        VK_IMAGE_TYPE_2D,
        device.GetColorFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    );

    CreateImageView(
        vkDevice,
        m_colorOutput.Image,
        &m_colorOutput.ImageView,
        VK_IMAGE_VIEW_TYPE_2D,
        device.GetColorFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    TransitionImageLayout(
        device.GetCommandBuffer(),
        m_colorOutput.Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    );

    m_ownOutput = true;
}

void PostProcessPass::ResizeOutput()
{
    if (!m_ownOutput)
    {
        return;
    }
    DestroyOutput();
    CreateOutput();
}

void PostProcessPass::PostRender()
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
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
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

void TonemapPass::Render(uint32_t frameIndex)
{
    PostProcessPass::PreRender();

    const auto  renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

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
        VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(VkDeviceAddress),
        renderer->GetShaderGlobalBuffer()->GetDeviceAddress()
    );

    vkCmdDraw(device.GetCommandBuffer(), 3, 1, 0, 0);

    PostProcessPass::PostRender();
}
}; // namespace yar
