#pragma once

#include "../ecs/camera.h"
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

enum RenderPipeline
{
    NONE,
    SKY,
    UNLIT,
    SHADED,
};

static const char* RenderPipelineNames[] = {"NONE", "SKY", "UNLIT", "SHADED"};

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

    std::shared_ptr<IBuffer> GetVertexBuffer(
        std::vector<float> positions,
        std::vector<float> normals,
        std::vector<float> tangents,
        std::vector<float> uvs
    )
    {
        std::vector<float> vertices = {};
        vertices.resize(positions.size() + normals.size() + tangents.size() + uvs.size());

        const auto vertexCount = positions / 3;
        const auto elementSize = sizeof(float) * (3 + 3 + 3 + 2);

        for (size_t v = 0; v < vertexCount; v++)
        {
            vertices[v + 0] = positions[0];
            vertices[v + 1] = positions[1];
            vertices[v + 2] = positions[2];

            vertices[v + 3] = normals[0];
            vertices[v + 4] = normals[1];
            vertices[v + 5] = normals[2];

            vertices[v + 6] = tangents[0];
            vertices[v + 7] = tangents[1];
            vertices[v + 8] = tangents[2];

            vertices[v + 9]  = uvs[0];
            vertices[v + 10] = uvs[1];
        }

        return CreateBuffer(BufferType::VertexBuffer, vertices.data(), elementSize, vertexCount);
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

    std::shared_ptr<Camera> m_camera;
};

extern std::shared_ptr<IRenderer> g_renderer;
}; // namespace yar
