#pragma once

#include "camera.h"
#include "ibuffer.h"
#include "isky.h"

#include <cstdint>
#include <memory>

namespace yar
{
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
};

enum RenderPipeline
{
    NONE,
    SKY,
    UNLIT,
    SHADED,
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

    virtual void SetCamera(std::shared_ptr<Camera> camera)
    {
        m_camera = camera;
    }

    virtual std::shared_ptr<Camera> GetCamera()
    {
        return m_camera;
    }

    virtual void CreateBuffer(
        std::shared_ptr<IBuffer>& buffer,
        BufferType                type,
        void*                     data,
        uint32_t                  elementSize,
        uint32_t                  elementCount
    ) = 0;

    virtual void  Resize()         = 0;
    virtual float GetAspect()      = 0;
    virtual void  Setup()          = 0;
    virtual void  Begin()          = 0;
    virtual void  PostProcess()    = 0;
    virtual void  Submit()         = 0;
    virtual void  Present()        = 0;
    virtual void  UpdateUniforms() = 0;
    virtual void  WaitForIdle()    = 0;

    virtual void BindPipeline(RenderPipeline pipe) = 0;

    virtual void BindDescriptor(uint32_t objectIndex) = 0;

    virtual void DrawWithBuffers(
        const std::shared_ptr<IBuffer> vertexBuffer,
        const std::shared_ptr<IBuffer> indexBuffer
    ) = 0;

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
    }

    virtual void  SetExposure(float exposure)    = 0;
    virtual float GetExposure()                  = 0;
    virtual void  SetContrast(float contrast)    = 0;
    virtual float GetContrast()                  = 0;
    virtual void  SetIBLStrength(float strength) = 0;
    virtual float GetIBLStrength()               = 0;

  protected:
    RenderStats m_renderStats;

    std::shared_ptr<Camera> m_camera;
};

extern std::shared_ptr<IRenderer> g_renderer;
}; // namespace yar
