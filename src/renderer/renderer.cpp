#include <imgui_impl_vulkan.h>
#include <memory>
#include <vulkan/vulkan_core.h>

#include "../log.h"
#include "../shader/compiler.h"
#include "data_types.h"
#include "renderer.h"

namespace yar
{

#define MAX_FRAMES_IN_FLIGHT 2

Renderer::Renderer(std::shared_ptr<Window> window) :
    m_instance(window),
    m_device(m_instance, MAX_FRAMES_IN_FLIGHT)
{
    LOG_INFO("Creating Renderer");

    auto uboBuffers = std::vector<std::shared_ptr<VulkanBuffer>>();
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uboBuffers.push_back(
            std::make_shared<VulkanBuffer>(UniformBuffer, Host, sizeof(UniformBufferObject), 1)
        );
    }
    m_descriptorSet = std::make_shared<VulkanDescriptorSet>(m_device, uboBuffers);

    // TODO: abstract away all the shader + pipeline setup
    ShaderCompiler compiler;
    size_t         size;
    const void*    spirv = compiler.GetSpirv("simple.slang", SHADER_ENTRY_PIXEL, size);
    if (!spirv)
    {
        throw std::runtime_error("failed to load shader 1");
    }
    auto moduleSimpleFrag = GetVulkanCreateInfo(spirv, size);
    spirv                 = compiler.GetSpirv("simple.slang", SHADER_ENTRY_VERTEX, size);
    if (!spirv)
    {
        throw std::runtime_error("failed to load shader 2");
    }
    auto moduleSimpleVert = GetVulkanCreateInfo(spirv, size);
    auto stageSimpleFrag =
        FillShaderStageCreateInfo(&moduleSimpleFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto stageSimpleVert = FillShaderStageCreateInfo(&moduleSimpleVert, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector simpleStages {stageSimpleFrag, stageSimpleVert};

    m_testPipeline =
        std::make_shared<VulkanPipeline<Vertex>>(m_device, m_descriptorSet, simpleStages);

    spirv = compiler.GetSpirv("sky.slang", SHADER_ENTRY_PIXEL, size);
    if (!spirv)
    {
        throw std::runtime_error("failed to load shader 3");
    }
    auto moduleSkyFrag = GetVulkanCreateInfo(spirv, size);
    spirv              = compiler.GetSpirv("sky.slang", SHADER_ENTRY_VERTEX, size);
    if (!spirv)
    {
        throw std::runtime_error("failed to load shader 4");
    }
    auto moduleSkyVert = GetVulkanCreateInfo(spirv, size);
    auto stageSkyFrag  = FillShaderStageCreateInfo(&moduleSkyFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto stageSkyVert  = FillShaderStageCreateInfo(&moduleSkyVert, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector skyStages {stageSkyFrag, stageSkyVert};

    m_skyPipeline = std::make_shared<VulkanPipeline<Vertex>>(m_device, m_descriptorSet, skyStages);
}

Renderer::~Renderer()
{
    LOG_INFO("Destroying Renderer");

    vkDeviceWaitIdle(m_device.GetVkDevice());

    m_testPipeline.reset();
    m_skyPipeline.reset();

    m_descriptorSet.reset();

    m_frameBuffers.clear();
}

void Renderer::SetWindow(std::shared_ptr<Window> window)
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

void Renderer::Begin()
{
    m_device.Begin();
}

void Renderer::Submit()
{
    m_device.Submit();

    // Not ideal but guarantees chunk buffers aren't freed too early.
    m_device.WaitForGraphicsIdle();
    m_frameBuffers.clear();
}

void Renderer::Present()
{
    m_device.Present();
}

void Renderer::UpdateUniforms(const std::shared_ptr<Camera> camera)
{
    auto currentFrame = m_device.GetCurrentFrame();
    auto ubo          = UniformBufferObject(camera);
    m_descriptorSet->UpdateUBO(currentFrame, &ubo);
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
    info.imInit.ImageCount                 = MAX_FRAMES_IN_FLIGHT;
    info.imInit.PipelineCache              = VK_NULL_HANDLE; // TODO
    info.imInit.PipelineInfoMain           = info.imPipeline;
    info.imInit.UseDynamicRendering        = true;
    info.imInit.MinImageCount              = MAX_FRAMES_IN_FLIGHT;
    info.imInit.Allocator                  = nullptr; // TODO vma?
    info.imInit.CheckVkResultFn            = ImGuiVkCheck;
    info.imInit.MinAllocationSize          = 1024 * 1024;
    info.imInit.CustomShaderVertCreateInfo = {};
    info.imInit.CustomShaderFragCreateInfo = {};
}

constexpr VkShaderModuleCreateInfo Renderer::GetVulkanCreateInfo(const void* data, size_t size)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = static_cast<const uint32_t*>(data);
    return createInfo;
}

constexpr VkPipelineShaderStageCreateInfo Renderer::FillShaderStageCreateInfo(
    VkShaderModuleCreateInfo* module,
    VkShaderStageFlagBits     stage
)
{
    VkPipelineShaderStageCreateInfo createInfo {};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = VK_NULL_HANDLE;
    createInfo.pName  = "main";
    createInfo.pNext  = module;
    return createInfo;
}
} // namespace yar
