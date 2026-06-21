#pragma once

#include <memory>
#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../components/camera.h"
#include "../components/rect.h"
#include "../window/window.h"
#include "buffer.h"
#include "data_types.h"
#include "vulkan/buffer.h"
#include "vulkan/common.h"
#include "vulkan/descriptor_set.h"
#include "vulkan/device.h"
#include "vulkan/instance.h"
#include "vulkan/pipeline.h"

namespace yar
{
#define MAX_FRAMES_IN_FLIGHT 2

struct RenderStats
{
    size_t MeshCount;
    size_t VertexCount;
    size_t IndexCount;
};

struct CullStats
{
    size_t MeshCount;
    size_t VertexCount;
    size_t IndexCount;
};

enum RenderPipeline
{
    NONE,
    UNLIT,
    SHADED,
};

class Renderer
{
  public:
    Renderer(std::shared_ptr<Window> window);
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    void SetWindow(std::shared_ptr<Window> window);

    void ResetViewport()
    {
        m_device.ResetViewport();
    }

    void SetViewport(Rect rect)
    {
        m_device.SetViewport(rect);
    }

    void  Resize();
    float GetAspect();
    void  Begin();
    void  Submit();
    void  Present();
    void  UpdateUniforms(const std::shared_ptr<Camera> camera);
    void  WaitForIdle();

    void* GetCommandBuffer()
    {
        return GetVkCommandBuffer();
    };

    void GetImGuiInfo(VulkanImGuiCreationInfo& info);

    VkCommandBuffer GetVkCommandBuffer() const
    {
        return m_device.GetCommandBuffer();
    }

    VkCommandBuffer GetTemporaryCommandBuffer()
    {
        return m_device.GetTemporaryCommandBuffer();
    }

    void SubmitTemporaryCommandBuffer(VkCommandBuffer buffer)
    {
        m_device.SubmitTemporaryCommandBuffer(buffer);
    }

    void CreateBuffer(
        std::shared_ptr<Buffer>& buffer,
        BufferType               bufferType,
        void*                    data,
        uint32_t                 elementSize,
        uint32_t                 elementCount
    )
    {
        auto hostBuffer = std::make_shared<VulkanBuffer>(
            m_device.GetVkDevice(),
            bufferType,
            Host,
            elementSize,
            elementCount
        );
        auto deviceBuffer = std::make_shared<VulkanBuffer>(
            m_device.GetVkDevice(),
            bufferType,
            Device,
            elementSize,
            elementCount
        );
        hostBuffer->Write(data, elementSize * elementCount);
        auto tempBuffer = GetTemporaryCommandBuffer();
        hostBuffer->CopyToDevice(tempBuffer, static_pointer_cast<Buffer>(deviceBuffer));
        SubmitTemporaryCommandBuffer(tempBuffer);
        buffer = static_pointer_cast<Buffer>(deviceBuffer);
    }

    void BindPipeline(RenderPipeline pipe)
    {
        if (m_currentPipeline == pipe)
        {
            return;
        }

        m_currentPipeline = pipe;

        auto commandBuffer = m_device.GetCommandBuffer();
        auto currentFrame  = m_device.GetCurrentFrame();

        switch (pipe)
        {
            case UNLIT:
            {
                m_pipelineUnlit->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
                vkCmdPushConstants(
                    commandBuffer,
                    m_pipelineUnlit->GetVkPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(VkDeviceAddress),
                    m_shaderGlobalBuffers[currentFrame]->GetDeviceAddress()
                );
                break;
            }

            case SHADED:
            {
                m_pipelineShaded->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
                vkCmdPushConstants(
                    commandBuffer,
                    m_pipelineShaded->GetVkPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(VkDeviceAddress),
                    m_shaderGlobalBuffers[currentFrame]->GetDeviceAddress()
                );
                break;
            }

            default:
            {
                throw std::runtime_error("unknown pipeline");
            }
        }
    }

    void SetModelMatrix(const Transform& trans)
    {
        const auto currentFrame  = m_device.GetCurrentFrame();
        const auto data          = ShaderObjectData {.model = trans.matrix};
        const auto objectIndex   = m_descriptorSet->AppendShaderObjectData(currentFrame, &data);
        auto       commandBuffer = GetVkCommandBuffer();
        switch (m_currentPipeline)
        {
            case UNLIT:
            {
                m_pipelineUnlit->BindDescriptor(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    currentFrame,
                    objectIndex
                );
                break;
            }

            case SHADED:
            {
                m_pipelineShaded->BindDescriptor(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    currentFrame,
                    objectIndex
                );
                break;
            }

            default:
            {
                LOG_ERROR("Tried to update descriptor with no pipeline");
                break;
            }
        }
    }

    void DrawWithBuffers(std::shared_ptr<Buffer> vertexBuffer, std::shared_ptr<Buffer> indexBuffer)
    {
        if (m_currentPipeline == RenderPipeline::NONE)
        {
            LOG_ERROR("Tried to draw with no pipeline");
            return;
        }

        auto commandBuffer = GetCommandBuffer();
        if (commandBuffer != nullptr)
        {
            vertexBuffer->Bind(commandBuffer);

            indexBuffer->Bind(commandBuffer);
            indexBuffer->Draw(commandBuffer, 0, 1);

            m_frameBuffers.push_back(vertexBuffer);
            m_frameBuffers.push_back(indexBuffer);

            m_renderStats.MeshCount++;
            m_renderStats.IndexCount += indexBuffer->GetElementCount();
            m_renderStats.VertexCount += vertexBuffer->GetElementCount();
        }
    }

    RenderStats GetRenderStats()
    {
        return m_renderStats;
    }

    CullStats GetCullStats()
    {
        return m_cullStats;
    }

    void ResetFrameStats()
    {
        m_renderStats.MeshCount   = 0;
        m_renderStats.IndexCount  = 0;
        m_renderStats.VertexCount = 0;
        m_cullStats.MeshCount     = 0;
        m_cullStats.IndexCount    = 0;
        m_cullStats.VertexCount   = 0;
    }

    void AddCulledMesh(std::shared_ptr<Buffer> vertexBuffer, std::shared_ptr<Buffer> indexBuffer)
    {
        m_cullStats.MeshCount++;
        m_cullStats.IndexCount += indexBuffer->GetElementCount();
        m_cullStats.VertexCount += vertexBuffer->GetElementCount();
    }

    VulkanDevice& GetDevice()
    {
        return m_device;
    }

  private:
    constexpr VkShaderModuleCreateInfo GetVulkanCreateInfo(const void* data, size_t size);

    constexpr VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
        VkShaderModuleCreateInfo* module,
        VkShaderStageFlagBits     stage
    );

    VulkanInstance m_instance;
    VulkanDevice   m_device;

    std::shared_ptr<VulkanDescriptorSet> m_descriptorSet;

    RenderPipeline m_currentPipeline;

    std::vector<std::shared_ptr<VulkanBuffer>>     m_shaderGlobalBuffers;
    std::vector<std::shared_ptr<ShaderGlobalData>> m_shaderGlobalData;

    std::shared_ptr<VulkanPipeline<VertexUnlit>>  m_pipelineUnlit;
    std::shared_ptr<VulkanPipeline<VertexShaded>> m_pipelineShaded;

    // Hold so we don't call Buffer destructor
    // while still in use by command buffer.
    std::vector<std::shared_ptr<Buffer>> m_frameBuffers;

    RenderStats m_renderStats;
    CullStats   m_cullStats;
};
} // namespace yar
