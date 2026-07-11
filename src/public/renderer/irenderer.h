#pragma once

#include "../ecs/camera.h"
#include "../material.h"
#include "ibuffer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace yar
{
#define MAX_FRAMES_IN_FLIGHT 1

struct RenderStats
{
    uint32_t NodeCount;
    size_t   VertexCount;
    size_t   IndexCount;
    uint32_t TextureCount;
    size_t   CulledNodeCount;
    size_t   CulledVertexCount;
    size_t   CulledIndexCount;
    double   SceneUpdateTime;
    double   SceneCullTime;
    double   SceneBatchTime;
    double   SceneSortTime;
    double   SceneDescriptorTime;
    double   SceneRenderTime;
    double   PostProcessTime;
    double   SetupTime;
    double   AcquireBlockTime;
    double   SubmitTime;
    double   PresentTime;
    double   PresentBlockTime;
};

class IRenderer
{
  public:
    IRenderer()          = default;
    virtual ~IRenderer() = default;

    IRenderer(const IRenderer&)            = delete;
    IRenderer(IRenderer&&)                 = delete;
    IRenderer& operator=(const IRenderer&) = delete;
    IRenderer& operator=(IRenderer&&)      = delete;

    virtual void SetCamera(Camera* camera)
    {
        m_camera = camera;
    }

    virtual const Camera* GetCamera()
    {
        return m_camera;
    }

    virtual std::shared_ptr<IBuffer> CreateBuffer(
        BufferType type,
        void*      data,
        uint32_t   elementSize,
        uint32_t   elementCount
    ) = 0;

    std::shared_ptr<IBuffer> GetIndexBuffer(std::vector<uint32_t> indices)
    {
        return CreateBuffer(
            BufferType::IndexBuffer,
            indices.data(),
            sizeof(uint32_t),
            static_cast<uint32_t>(indices.size())
        );
    }

    virtual void  Resize()         = 0;
    virtual float GetAspect()      = 0;
    virtual void  Setup()          = 0;
    virtual void  Begin()          = 0;
    virtual void  PostProcess()    = 0;
    virtual void  BeginUI()        = 0;
    virtual void  EndUI()          = 0;
    virtual void  Submit()         = 0;
    virtual void  Present()        = 0;
    virtual void  UpdateUniforms() = 0;
    virtual void  WaitForIdle()    = 0;

    virtual void BindPipeline(const VkPipeline pipeline, const VkPipelineLayout layout) = 0;

    virtual void BindDescriptor(uint32_t objectIndex, const VkPipelineLayout layout) = 0;

    virtual void DrawWithBuffers(const std::shared_ptr<IBuffer> indexBuffer) = 0;

    RenderStats& GetRenderStats()
    {
        return m_renderStats;
    }

    void ResetFrameStats()
    {
        m_renderStats.NodeCount           = 0;
        m_renderStats.VertexCount         = 0;
        m_renderStats.IndexCount          = 0;
        m_renderStats.TextureCount        = 0;
        m_renderStats.CulledNodeCount     = 0;
        m_renderStats.CulledVertexCount   = 0;
        m_renderStats.CulledIndexCount    = 0;
        m_renderStats.SceneUpdateTime     = 0.0;
        m_renderStats.SceneCullTime       = 0.0;
        m_renderStats.SceneBatchTime      = 0.0;
        m_renderStats.SceneSortTime       = 0.0;
        m_renderStats.SceneDescriptorTime = 0.0;
        m_renderStats.SceneRenderTime     = 0.0;
        m_renderStats.PostProcessTime     = 0.0;
        m_renderStats.SetupTime           = 0.0;
        m_renderStats.AcquireBlockTime    = 0.0;
        m_renderStats.SubmitTime          = 0.0;
        m_renderStats.PresentTime         = 0.0;
        m_renderStats.PresentBlockTime    = 0.0;
    }

    virtual void  SetExposure(float exposure)    = 0;
    virtual float GetExposure()                  = 0;
    virtual void  SetContrast(float contrast)    = 0;
    virtual float GetContrast()                  = 0;
    virtual void  SetIBLStrength(float strength) = 0;
    virtual float GetIBLStrength()               = 0;

    virtual const char* GetPresentMode() = 0;

    virtual uint32_t GetSwapchainImageCount() = 0;

  protected:
    RenderStats m_renderStats;

    Camera* m_camera;
};

extern std::shared_ptr<IRenderer> g_renderer;
}; // namespace yar
