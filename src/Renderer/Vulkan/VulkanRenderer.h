#pragma once

#include <memory>
#include <stdexcept>

#include <imgui_impl_vulkan.h>

#include "../Renderer.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"

namespace drive
{
class VulkanRenderer final : public Renderer
{
  public:
    VulkanRenderer(std::shared_ptr<Window> window);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&)            = delete;
    VulkanRenderer(VulkanRenderer&&)                 = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(VulkanRenderer&&)      = delete;

    void SetWindow(std::shared_ptr<Window> window) override;

    void ResetViewport() override
    {
        m_device.ResetViewport();
    }

    void SetViewport(Rect rect) override
    {
        m_device.SetViewport(rect);
    }

    void  ClearViewport() override;
    void  Resize() override;
    float GetAspect() override;
    void  Begin() override;
    void  Submit() override;
    void  Present() override;
    void  UpdateUniforms(const std::shared_ptr<Camera> camera) override;
    void  WaitForIdle() override;

    void* GetCommandBuffer() override
    {
        return GetVkCommandBuffer();
    };

    RendererType Type() const override
    {
        return RendererType::VULKAN;
    }

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
    ) override
    {
        auto hostBuffer =
            std::make_shared<VulkanBuffer>(bufferType, Host, elementSize, elementCount);
        auto deviceBuffer =
            std::make_shared<VulkanBuffer>(bufferType, Device, elementSize, elementCount);
        hostBuffer->Write(data, elementSize * elementCount);
        auto tempBuffer = GetTemporaryCommandBuffer();
        hostBuffer->CopyToDevice(tempBuffer, static_pointer_cast<Buffer>(deviceBuffer));
        SubmitTemporaryCommandBuffer(tempBuffer);
        buffer = static_pointer_cast<Buffer>(deviceBuffer);
    }

    void BindPipeline(RenderPipeline pipe) override
    {
        auto commandBuffer = m_device.GetCommandBuffer();
        auto currentFrame  = m_device.GetCurrentFrame();

        switch (pipe)
        {
            case TEST:
            {
                m_testPipeline->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;
            }

            case TERRAIN:
            {
                m_terrainPipeline
                    ->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;
            }

            case FULLSCREEN:
            {
                m_fullscreenPipeline
                    ->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;
            }

            case SKY:
            {
                m_skyPipeline->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;
            }

            default:
            {
                throw std::runtime_error("unknown pipeline");
            }
        }
    }

  private:
    VkShaderModule& CreateShaderModule(VkShaderModuleCreateInfo createInfo);
    constexpr VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
        VkShaderModule&       module,
        VkShaderStageFlagBits stage
    );

    VulkanInstance m_instance;
    VulkanDevice   m_device;

    std::shared_ptr<VulkanDescriptorSet> m_descriptorSet;
    std::vector<VkShaderModule>          m_vkShaderModules;

    std::shared_ptr<VulkanPipeline<Vertex_P_C>>   m_testPipeline;
    std::shared_ptr<VulkanPipeline<Vertex_P_N_C>> m_terrainPipeline;
    std::shared_ptr<VulkanPipeline<VertexEmpty>>  m_fullscreenPipeline;
    std::shared_ptr<VulkanPipeline<Vertex_P>>     m_skyPipeline;
};
} // namespace drive
