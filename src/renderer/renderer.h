#pragma once

#include <memory>
#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../components/camera.h"
#include "../components/mesh.h"
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

enum RenderPipeline
{
    TEST,
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
        auto commandBuffer = m_device.GetCommandBuffer();
        auto currentFrame  = m_device.GetCurrentFrame();

        switch (pipe)
        {
            case TEST:
            {
                m_testPipeline->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                vkCmdPushConstants(
                    commandBuffer,
                    m_testPipeline->GetVkPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
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

    void DrawWithBuffers(std::shared_ptr<Buffer> vertexBuffer, std::shared_ptr<Buffer> indexBuffer)
    {
        auto commandBuffer = GetCommandBuffer();
        if (commandBuffer != nullptr)
        {
            vertexBuffer->Bind(commandBuffer);

            indexBuffer->Bind(commandBuffer);
            indexBuffer->Draw(commandBuffer, 0, 1);

            m_frameBuffers.push_back(vertexBuffer);
            m_frameBuffers.push_back(indexBuffer);

            m_stats.MeshCount++;
            m_stats.IndexCount += indexBuffer->GetElementCount();
            m_stats.VertexCount += vertexBuffer->GetElementCount();
        }
    }

    RenderStats GetStats()
    {
        return m_stats;
    }

    void ResetFrameStats()
    {
        m_stats.MeshCount   = 0;
        m_stats.IndexCount  = 0;
        m_stats.VertexCount = 0;
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

    std::vector<std::shared_ptr<VulkanBuffer>>     m_shaderGlobalBuffers;
    std::vector<std::shared_ptr<ShaderGlobalData>> m_shaderGlobalData;

    std::shared_ptr<VulkanPipeline<VertexUnlit>> m_testPipeline;

    // Hold so we don't call Buffer destructor
    // while still in use by command buffer.
    std::vector<std::shared_ptr<Buffer>> m_frameBuffers;

    RenderStats m_stats;
};
} // namespace yar
