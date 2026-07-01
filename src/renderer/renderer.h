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
#include "data_types.h"
#include "vulkan/buffer.h"
#include "vulkan/common.h"
#include "vulkan/descriptor_set.h"
#include "vulkan/device.h"
#include "vulkan/image.h"
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
    double SortTime;
};

struct CullStats
{
    size_t MeshCount;
    size_t VertexCount;
    size_t IndexCount;
    double CullTime;
};

enum RenderPipeline
{
    NONE,
    SKY,
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

    void SubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
    {
        m_device.SubmitTemporaryCommandBuffer(commandBuffer);
    }

    void CreateBuffer(
        std::shared_ptr<Buffer>& buffer,
        BufferType               bufferType,
        void*                    data,
        uint32_t                 elementSize,
        uint32_t                 elementCount
    )
    {
        auto vkBuffer = std::make_shared<Buffer>(
            m_device.GetVkDevice(),
            bufferType,
            SecretThirdOption,
            elementSize,
            elementCount
        );
        std::memcpy(vkBuffer->GetAllocationInfo().pMappedData, data, elementSize * elementCount);
        buffer = static_pointer_cast<Buffer>(vkBuffer);
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

    void CullMeshes(
        std::shared_ptr<Camera>                           camera,
        std::vector<std::shared_ptr<Mesh<VertexShaded>>>& meshes
    )
    {
        const auto                                       startTime = Time::Now();
        std::vector<std::shared_ptr<Mesh<VertexShaded>>> visible   = {};
        for (const auto& mesh : meshes)
        {
            mesh->FrustumCull(camera);
            if (mesh->Culled)
            {
                AddCulledMesh(mesh->GetVertexBuffer(), mesh->GetIndexBuffer());
            }
            else
            {
                visible.push_back(mesh);
            }
        }
        meshes               = visible;
        m_cullStats.CullTime = Time::Now() - startTime;
    }

    void SortMeshes(
        std::shared_ptr<Camera>                           camera,
        std::vector<std::shared_ptr<Mesh<VertexShaded>>>& meshes
    )
    {
        const auto startTime = Time::Now();

        const auto cameraPos = camera->transform.GetPosition();

        std::sort(
            meshes.begin(),
            meshes.end(),
            [&cameraPos](
                std::shared_ptr<Mesh<VertexShaded>> a,
                std::shared_ptr<Mesh<VertexShaded>> b
            ) {
                const auto queueA = a->GetMaterial()->GetQueue();
                const auto queueB = b->GetMaterial()->GetQueue();

                if (queueA < queueB)
                {
                    return true;
                }

                if (queueA == queueB)
                {
                    const auto distA = glm::length(cameraPos - a->GetAABB().center);
                    const auto distB = glm::length(cameraPos - b->GetAABB().center);
                    return distA < distB;
                }

                return false;
            }
        );

        m_renderStats.SortTime = Time::Now() - startTime;
    }

    void UpdateDescriptor(const std::vector<std::shared_ptr<Mesh<VertexShaded>>>& meshes)
    {
        const auto currentFrame = m_device.GetCurrentFrame();
        m_descriptorSet->Update(currentFrame, meshes);
    }

    void BindDescriptor(uint32_t objectIndex)
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
        m_renderStats.SortTime    = 0.0;
        m_cullStats.MeshCount     = 0;
        m_cullStats.IndexCount    = 0;
        m_cullStats.VertexCount   = 0;
        m_cullStats.CullTime      = 0.0;
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

    bool IsDrawing() const
    {
        return m_drawing;
    }

    void SetIBL(
        std::shared_ptr<Texture> texColor,
        std::shared_ptr<Texture> texLUT,
        std::shared_ptr<Texture> texDiffuse,
        std::shared_ptr<Texture> texSpecular
    )
    {
        m_iblColor    = texColor;
        m_iblLUT      = texLUT;
        m_iblDiffuse  = texDiffuse;
        m_iblSpecular = texSpecular;
        m_descriptorSet->SetIBL(texColor, texLUT, texDiffuse, texSpecular);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetIBLMips(
                static_cast<float>(m_iblSpecular->GetImage()->GetMips())
            );
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

    void SetExposure(float exposure)
    {
        LOG_INFO("Set exposure to {}", exposure);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetExposure(exposure);
        }
    }

    float GetExposure()
    {
        return m_shaderGlobalData[0]->GetExposure();
    }

    void SetContrast(float contrast)
    {
        LOG_INFO("Set contrast to {}", contrast);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetContrast(contrast);
        }
    }

    float GetContrast()
    {
        return m_shaderGlobalData[0]->GetContrast();
    }

    void SetIBLStrength(float strength)
    {
        LOG_INFO("Set IBL strength to {}", strength);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_shaderGlobalData[i]->SetIBLStrength(strength);
        }
    }

    float GetIBLStrength()
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

    std::shared_ptr<Texture> m_iblColor;
    std::shared_ptr<Texture> m_iblLUT;
    std::shared_ptr<Texture> m_iblDiffuse;
    std::shared_ptr<Texture> m_iblSpecular;

    RenderStats m_renderStats;
    CullStats   m_cullStats;

    bool m_drawing;
};
} // namespace yar
