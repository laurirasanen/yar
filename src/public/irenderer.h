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
    virtual void  Begin()          = 0;
    virtual void  Submit()         = 0;
    virtual void  Present()        = 0;
    virtual void  UpdateUniforms() = 0;
    virtual void  WaitForIdle()    = 0;

    virtual void BindPipeline(RenderPipeline pipe) = 0;

    virtual void BindDescriptor(uint32_t objectIndex) = 0;

    virtual void DrawWithBuffers(
        std::shared_ptr<IBuffer> vertexBuffer,
        std::shared_ptr<IBuffer> indexBuffer
    ) = 0;

    RenderStats& GetRenderStats()
    {
        return m_renderStats;
    }

    CullStats& GetCullStats()
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

    void AddCulledMesh(std::shared_ptr<IBuffer> vertexBuffer, std::shared_ptr<IBuffer> indexBuffer)
    {
        m_cullStats.MeshCount++;
        m_cullStats.IndexCount += indexBuffer->GetElementCount();
        m_cullStats.VertexCount += vertexBuffer->GetElementCount();
    }

    virtual void  SetExposure(float exposure)    = 0;
    virtual float GetExposure()                  = 0;
    virtual void  SetContrast(float contrast)    = 0;
    virtual float GetContrast()                  = 0;
    virtual void  SetIBLStrength(float strength) = 0;
    virtual float GetIBLStrength()               = 0;

  protected:
    RenderStats m_renderStats;
    CullStats   m_cullStats;

    std::shared_ptr<Camera> m_camera;
};

extern std::shared_ptr<IRenderer> g_renderer;
}; // namespace yar
