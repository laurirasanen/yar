#include <memory>

#include <glm/ext/matrix_transform.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../public/log.h"
#include "../shader/compiler.h"
#include "data_types.h"
#include "renderer.h"
#include "src/public/irenderer.h"

namespace yar
{
std::shared_ptr<Scene>          g_scene;
std::shared_ptr<ShaderCompiler> g_shaderCompiler;

Renderer::Renderer(std::shared_ptr<SDLWindow> window) : m_instance(window), m_device(m_instance)
{
    LOG_INFO("Creating Renderer");
}

Renderer::~Renderer()
{
    LOG_INFO("Destroying Renderer");

    vkDeviceWaitIdle(m_device.GetVkDevice());

    g_scene.reset();

    m_pipelineUnlit.reset();
    m_pipelineShaded.reset();

    m_descriptorSet.reset();

    m_frameBuffers.clear();
}

void Renderer::SetWindow(std::shared_ptr<SDLWindow> window)
{
    LOG_INFO("Setting window");
    m_instance.SetWindow(window);
    Resize();
}

void Renderer::Resize()
{
    LOG_INFO("Resizing");
    m_device.ResizeFramebuffer();
}

float Renderer::GetAspect()
{
    return m_device.GetSwapchainAspect();
}

void Renderer::Setup()
{
    m_shaderGlobalBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_shaderGlobalData.resize(MAX_FRAMES_IN_FLIGHT);
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_shaderGlobalBuffers[i] = std::make_shared<Buffer>(
            m_device.GetVkDevice(),
            ShaderDataBuffer,
            SecretThirdOption,
            sizeof(ShaderGlobalData),
            1
        );
        m_shaderGlobalData[i] = std::make_shared<ShaderGlobalData>();
    }

    m_descriptorSet = std::make_shared<DescriptorSet>(MAX_FRAMES_IN_FLIGHT);
    m_descriptorSet->Alloc();

    g_shaderCompiler = std::make_shared<ShaderCompiler>();

    // TODO: abstract away to materials?
    size_t size;

    {
        const void* spirv = g_shaderCompiler->GetSpirv("sky.slang", SHADER_ENTRY_PIXEL, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load sky fragment shader");
        }

        auto fragModule = GetShaderCreateInfo(spirv, size);

        spirv = g_shaderCompiler->GetSpirv("sky.slang", SHADER_ENTRY_VERTEX, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load sky vertex shader");
        }

        auto vertModule = GetShaderCreateInfo(spirv, size);
        auto shaderFrag = FillShaderStageCreateInfo(&fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto shaderVert = FillShaderStageCreateInfo(&vertModule, VK_SHADER_STAGE_VERTEX_BIT);
        std::vector stages {shaderFrag, shaderVert};

        m_pipelineSky = std::make_shared<VulkanPipeline<VertexSky>>(
            m_device.GetVkDevice(),
            stages,
            m_descriptorSet->GetLayouts(),
            m_device.GetColorFormat(),
            m_device.GetDepthFormat()
        );
    }

    {
        const void* spirv = g_shaderCompiler->GetSpirv("unlit.slang", SHADER_ENTRY_PIXEL, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load unlit fragment shader");
        }

        auto fragModule = GetShaderCreateInfo(spirv, size);

        spirv = g_shaderCompiler->GetSpirv("unlit.slang", SHADER_ENTRY_VERTEX, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load unlit vertex shader");
        }

        auto vertModule = GetShaderCreateInfo(spirv, size);
        auto shaderFrag = FillShaderStageCreateInfo(&fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto shaderVert = FillShaderStageCreateInfo(&vertModule, VK_SHADER_STAGE_VERTEX_BIT);
        std::vector stages {shaderFrag, shaderVert};

        m_pipelineUnlit = std::make_shared<VulkanPipeline<VertexUnlit>>(
            m_device.GetVkDevice(),
            stages,
            m_descriptorSet->GetLayouts(),
            m_device.GetColorFormat(),
            m_device.GetDepthFormat()
        );
    }

    {
        const void* spirv = g_shaderCompiler->GetSpirv("shaded.slang", SHADER_ENTRY_PIXEL, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load shaded fragment shader");
        }

        auto fragModule = GetShaderCreateInfo(spirv, size);

        spirv = g_shaderCompiler->GetSpirv("shaded.slang", SHADER_ENTRY_VERTEX, size);
        if (!spirv)
        {
            throw std::runtime_error("failed to load shaded vertex shader");
        }

        auto vertModule = GetShaderCreateInfo(spirv, size);
        auto shaderFrag = FillShaderStageCreateInfo(&fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);
        auto shaderVert = FillShaderStageCreateInfo(&vertModule, VK_SHADER_STAGE_VERTEX_BIT);
        std::vector stages {shaderFrag, shaderVert};

        m_pipelineShaded = std::make_shared<VulkanPipeline<VertexShaded>>(
            m_device.GetVkDevice(),
            stages,
            m_descriptorSet->GetLayouts(),
            m_device.GetColorFormat(),
            m_device.GetDepthFormat()
        );
    }

    g_scene = std::make_shared<Scene>();
}

void Renderer::Begin()
{
    const auto startTime = Time::Now();
    m_descriptorSet->NewFrame();
    m_currentPipeline = RenderPipeline::NONE;
    m_device.Begin();
    m_renderStats.SetupTime = Time::Now() - startTime;
}

void Renderer::PostProcess()
{
    const auto startTime = Time::Now();
    m_device.PostProcess();
    m_renderStats.PostProcessTime = Time::Now() - startTime;
}

void Renderer::Submit()
{
    const auto startTime = Time::Now();
    m_device.Submit();
    m_frameBuffers.clear();
    m_renderStats.SubmitTime = Time::Now() - startTime;
}

void Renderer::Present()
{
    const auto startTime = Time::Now();
    m_device.Present();
    m_renderStats.PresentTime = Time::Now() - startTime;
}

void Renderer::UpdateUniforms()
{
    auto currentFrame = m_device.GetCurrentFrame();

    m_shaderGlobalData[currentFrame]->Update(m_camera);

    std::memcpy(
        m_shaderGlobalBuffers[currentFrame]->GetAllocationInfo().pMappedData,
        m_shaderGlobalData[currentFrame].get(),
        sizeof(ShaderGlobalData)
    );
}

void Renderer::WaitForIdle()
{
    vkDeviceWaitIdle(m_device.GetVkDevice());
}

void Renderer::GetImGuiInfo(VulkanImGuiCreationInfo& info)
{
    info.vkColor         = m_device.GetSwapchainImageFormat();
    VkFormat depthFormat = m_device.GetDepthFormat();

    info.vkPipeline                         = {};
    info.vkPipeline.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    info.vkPipeline.pNext                   = VK_NULL_HANDLE;
    info.vkPipeline.colorAttachmentCount    = 1;
    info.vkPipeline.pColorAttachmentFormats = &info.vkColor;
    info.vkPipeline.depthAttachmentFormat   = depthFormat;
    info.vkPipeline.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    info.imPipeline                             = {};
    info.imPipeline.RenderPass                  = VK_NULL_HANDLE;
    info.imPipeline.Subpass                     = 0;
    info.imPipeline.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    info.imPipeline.ExtraDynamicStates          = {};
    info.imPipeline.PipelineRenderingCreateInfo = info.vkPipeline;

    info.imInit                            = {};
    info.imInit.ApiVersion                 = VK_API_VERSION_1_4;
    info.imInit.Instance                   = m_instance.GetVkInstance();
    info.imInit.PhysicalDevice             = m_device.GetVkPhysicalDevice();
    info.imInit.Device                     = m_device.GetVkDevice();
    info.imInit.QueueFamily                = m_device.GetGraphicsQueueIndex();
    info.imInit.Queue                      = m_device.GetGraphicsQueue();
    info.imInit.DescriptorPool             = m_device.GetImGuiDescriptorPool();
    info.imInit.DescriptorPoolSize         = 0;
    info.imInit.MinImageCount              = 2;
    info.imInit.ImageCount                 = MAX(2, MAX_FRAMES_IN_FLIGHT);
    info.imInit.PipelineCache              = VK_NULL_HANDLE; // TODO
    info.imInit.PipelineInfoMain           = info.imPipeline;
    info.imInit.UseDynamicRendering        = true;
    info.imInit.Allocator                  = nullptr; // TODO vma
    info.imInit.CheckVkResultFn            = ImGuiVkCheck;
    info.imInit.MinAllocationSize          = 1024 * 1024;
    info.imInit.CustomShaderVertCreateInfo = {};
    info.imInit.CustomShaderFragCreateInfo = {};
}
} // namespace yar
