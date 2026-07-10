#pragma once

#include <memory>
#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../public/geometry.h"
#include "../public/renderer/irenderer.h"
#include "../window/window.h"
#include "../world/sky.h"
#include "buffer.h"
#include "common.h"
#include "data_types.h"
#include "descriptor_set.h"
#include "device.h"
#include "image.h"
#include "instance.h"
#include "mesh.h"
#include "pipeline.h"
#include "scene.h"

#include <memory>

namespace yar
{
class Renderer : public IRenderer
{
  public:
    Renderer(std::shared_ptr<SDLWindow> window);
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    void SetWindow(std::shared_ptr<SDLWindow> window);

    void ResetViewport()
    {
        m_device.ResetViewport();
    }

    void SetViewport(Rect rect)
    {
        m_device.SetViewport(rect);
    }

    void Resize() override;

    float GetAspect() override;

    void Setup() override;

    void Begin() override;

    void BeginUI() override
    {
        m_instance.BeginDebugLabel(m_device.GetCommandBuffer(), "UI", {0.5f, 1.0f, 0.5f, 0.8f});
        m_device.BeginUI();
    }

    void EndUI() override
    {
        m_device.EndUI();
        m_instance.EndDebugLabel(m_device.GetCommandBuffer());
    }

    void PostProcess() override;

    void Submit() override;

    void Present() override;

    void UpdateUniforms() override;

    void WaitForIdle() override;

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

    void SubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
    {
        m_device.SubmitTemporaryCommandBuffer(commandBuffer);
    }

    std::shared_ptr<IBuffer> CreateBuffer(
        BufferType bufferType,
        void*      data,
        uint32_t   elementSize,
        uint32_t   elementCount
    ) override
    {
        auto vkBuffer = std::make_shared<Buffer>(
            m_device.GetVkDevice(),
            bufferType,
            SecretThirdOption,
            elementSize,
            elementCount
        );
        std::memcpy(vkBuffer->GetAllocationInfo().pMappedData, data, elementSize * elementCount);
        return vkBuffer;
    }

    void BindPipeline(RenderPipeline pipe) override
    {
        if (m_currentPipeline == pipe)
        {
            return;
        }

        m_currentPipeline = pipe;

        auto             commandBuffer = m_device.GetCommandBuffer();
        auto             currentFrame  = m_device.GetCurrentFrame();
        VkPipeline       pipeline;
        VkPipelineLayout layout;

        switch (pipe)
        {
            case SKY:
            {
                pipeline = m_pipelineSky->GetVkPipeline();
                layout   = m_pipelineSky->GetVkPipelineLayout();
                break;
            }

            case UNLIT:
            {
                pipeline = m_pipelineUnlit->GetVkPipeline();
                layout   = m_pipelineUnlit->GetVkPipelineLayout();
                break;
            }

            case SHADED:
            {
                pipeline = m_pipelineShaded->GetVkPipeline();
                layout   = m_pipelineShaded->GetVkPipelineLayout();
                break;
            }

            default:
            {
                throw std::runtime_error("unknown pipeline");
            }
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdPushConstants(
            commandBuffer,
            layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(VkDeviceAddress),
            m_shaderGlobalBuffers[currentFrame]->GetDeviceAddress()
        );
    }

    std::shared_ptr<Buffer> GetShaderGlobalBuffer()
    {
        auto currentFrame = m_device.GetCurrentFrame();
        return m_shaderGlobalBuffers[currentFrame];
    }

    void UpdateDescriptor(const std::vector<std::shared_ptr<IRenderNode>>& nodes)
    {
        const auto currentFrame = m_device.GetCurrentFrame();
        m_descriptorSet->Update(currentFrame, nodes);
    }

    void BindDescriptor(uint32_t objectIndex) override
    {
        const auto       currentFrame  = m_device.GetCurrentFrame();
        auto             commandBuffer = GetVkCommandBuffer();
        VkPipelineLayout layout;

        switch (m_currentPipeline)
        {
            case SKY:
            {
                layout = m_pipelineSky->GetVkPipelineLayout();
                break;
            }

            case UNLIT:
            {
                layout = m_pipelineUnlit->GetVkPipelineLayout();
                break;
            }

            case SHADED:
            {
                layout = m_pipelineShaded->GetVkPipelineLayout();
                break;
            }

            default:
            {
                throw std::runtime_error("Tried to update descriptor with no pipeline");
            }
        }

        m_descriptorSet->Bind(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout,
            currentFrame,
            objectIndex
        );
    }

    void DrawWithBuffers(
        const std::shared_ptr<IBuffer> vertexBuffer,
        const std::shared_ptr<IBuffer> indexBuffer
    ) override
    {
        if (m_currentPipeline == RenderPipeline::NONE)
        {
            LOG_ERROR("Tried to draw with no pipeline");
            return;
        }

        auto vertex = static_pointer_cast<Buffer>(vertexBuffer);
        auto index  = static_pointer_cast<Buffer>(indexBuffer);

        auto commandBuffer = GetCommandBuffer();
        if (commandBuffer != nullptr)
        {
            vertex->Bind(commandBuffer);

            index->Bind(commandBuffer);
            index->Draw(commandBuffer, 0, 1);

            m_frameBuffers.push_back(vertex);
            m_frameBuffers.push_back(index);

            m_renderStats.NodeCount++;
            m_renderStats.IndexCount += index->GetElementCount();
            m_renderStats.VertexCount += vertex->GetElementCount();
        }
    }

    VulkanDevice& GetDevice()
    {
        return m_device;
    }

    void SetSky(std::shared_ptr<ISky> sky)
    {
        m_descriptorSet->SetSky(sky);

        auto mips = sky->GetSpecular()->GetImage()->GetMips();

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetIBLMips(static_cast<float>(mips));
        }
    }

    void SetExposure(float exposure) override
    {
        LOG_INFO("Set exposure to {}", exposure);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetExposure(exposure);
        }
    }

    float GetExposure() override
    {
        return m_shaderGlobalData[0]->GetExposure();
    }

    void SetContrast(float contrast) override
    {
        LOG_INFO("Set contrast to {}", contrast);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetContrast(contrast);
        }
    }

    float GetContrast() override
    {
        return m_shaderGlobalData[0]->GetContrast();
    }

    void SetIBLStrength(float strength) override
    {
        LOG_INFO("Set IBL strength to {}", strength);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetIBLStrength(strength);
        }
    }

    float GetIBLStrength() override
    {
        return m_shaderGlobalData[0]->GetIBLStrength();
    }

    void BeginDebugLabel(const char* name, glm::vec4 color)
    {
        BeginDebugLabel(m_device.GetCommandBuffer(), name, color);
    }

    void BeginDebugLabel(VkCommandBuffer commandBuffer, const char* name, glm::vec4 color)
    {
        m_instance.BeginDebugLabel(commandBuffer, name, color);
    }

    void EndDebugLabel()
    {
        EndDebugLabel(m_device.GetCommandBuffer());
    }

    void EndDebugLabel(VkCommandBuffer commandBuffer)
    {
        m_instance.EndDebugLabel(commandBuffer);
    }

    void SetDebugName(VkObjectType objectType, uint64_t objectHandle, const char* name)
    {
        m_instance.SetDebugName(m_device.GetVkDevice(), objectType, objectHandle, name);
    }

    const char* GetPresentMode() override
    {
        return PresentModeName(m_device.GetPresentMode());
    }

    uint32_t GetSwapchainImageCount() override
    {
        return m_device.GetSwapchainImageCount();
    }

  private:
    VulkanInstance m_instance;
    VulkanDevice   m_device;

    std::shared_ptr<DescriptorSet> m_descriptorSet;

    std::vector<std::shared_ptr<Buffer>>           m_shaderGlobalBuffers;
    std::vector<std::shared_ptr<ShaderGlobalData>> m_shaderGlobalData;

    // Hold so we don't call Buffer destructor
    // while still in use by command buffer.
    std::vector<std::shared_ptr<Buffer>> m_frameBuffers;
};
} // namespace yar
