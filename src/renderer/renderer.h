#pragma once

#include <memory>
#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../public/geometry.h"
#include "../public/irenderer.h"
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
#define MAX_FRAMES_IN_FLIGHT 2

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

    void Begin() override;

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

    void CreateBuffer(
        std::shared_ptr<IBuffer>& buffer,
        BufferType                bufferType,
        void*                     data,
        uint32_t                  elementSize,
        uint32_t                  elementCount
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
        buffer = vkBuffer;
    }

    void BindPipeline(RenderPipeline pipe) override
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
            case SKY:
            {
                m_pipelineSky->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
                vkCmdPushConstants(
                    commandBuffer,
                    m_pipelineSky->GetVkPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(VkDeviceAddress),
                    m_shaderGlobalBuffers[currentFrame]->GetDeviceAddress()
                );
                break;
            }

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

    void UpdateDescriptor(const std::vector<std::shared_ptr<INode>>& nodes)
    {
        const auto currentFrame = m_device.GetCurrentFrame();
        m_descriptorSet->Update(currentFrame, nodes);
    }

    void BindDescriptor(uint32_t objectIndex) override
    {
        const auto currentFrame  = m_device.GetCurrentFrame();
        auto       commandBuffer = GetVkCommandBuffer();
        switch (m_currentPipeline)
        {
            case SKY:
            {
                m_pipelineSky->BindDescriptor(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    currentFrame,
                    objectIndex
                );
                break;
            }

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

    void DrawWithBuffers(
        std::shared_ptr<IBuffer> vertexBuffer,
        std::shared_ptr<IBuffer> indexBuffer
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

            m_renderStats.MeshCount++;
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

    void SetMissingTexture(TextureType type, std::shared_ptr<Texture> tex)
    {
        switch (type)
        {
            case TextureType::TEX_ALBEDO:
            {
                m_missingAlbedo = tex;
                break;
            }

            case TextureType::TEX_ORM:
            {
                m_missingMetalness = tex;
                break;
            }

            case TextureType::TEX_NORMAL:
            {
                m_missingNormal = tex;
                break;
            }

            case TextureType::TEX_EMISSIVE:
            {
                m_missingEmissive = tex;
                break;
            }

            default:
            {
                throw std::runtime_error("Unhandled texture type");
            }
        }
    }

    std::shared_ptr<Texture> GetMissingTexture(TextureType type)
    {
        switch (type)
        {
            case TextureType::TEX_ALBEDO:
            {
                return m_missingAlbedo;
            }

            case TextureType::TEX_ORM:
            {
                return m_missingMetalness;
            }

            case TextureType::TEX_NORMAL:
            {
                return m_missingNormal;
            }

            case TextureType::TEX_EMISSIVE:
            {
                return m_missingEmissive;
            }

            default:
            {
                throw std::runtime_error("Unhandled texture type");
            }
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

  private:
    constexpr VkShaderModuleCreateInfo GetVulkanCreateInfo(const void* data, size_t size);

    constexpr VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
        VkShaderModuleCreateInfo* module,
        VkShaderStageFlagBits     stage
    );

    VulkanInstance m_instance;
    VulkanDevice   m_device;

    std::shared_ptr<DescriptorSet> m_descriptorSet;

    RenderPipeline m_currentPipeline;

    std::vector<std::shared_ptr<Buffer>>           m_shaderGlobalBuffers;
    std::vector<std::shared_ptr<ShaderGlobalData>> m_shaderGlobalData;

    std::shared_ptr<VulkanPipeline<VertexSky>>    m_pipelineSky;
    std::shared_ptr<VulkanPipeline<VertexUnlit>>  m_pipelineUnlit;
    std::shared_ptr<VulkanPipeline<VertexShaded>> m_pipelineShaded;

    // Hold so we don't call Buffer destructor
    // while still in use by command buffer.
    std::vector<std::shared_ptr<Buffer>> m_frameBuffers;

    std::shared_ptr<Texture> m_missingAlbedo;
    std::shared_ptr<Texture> m_missingMetalness;
    std::shared_ptr<Texture> m_missingNormal;
    std::shared_ptr<Texture> m_missingEmissive;
};
} // namespace yar
