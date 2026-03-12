#pragma once

#include <memory>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "../Components/Camera.h"
#include "../Components/Rect.h"
#include "../Window/Window.h"
#include "Buffer.h"

namespace drive
{
enum RendererType
{
    EMPTY,
    VULKAN,
};

enum RenderPipeline
{
    TEST,
    TERRAIN,
    FULLSCREEN,
    SKY,
};

class Renderer
{
  public:
    Renderer() = default;
    virtual ~Renderer() {};

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    virtual void         SetWindow(std::shared_ptr<Window> window)            = 0;
    virtual void         ResetViewport()                                      = 0;
    virtual void         SetViewport(Rect rect)                               = 0;
    virtual void         ClearViewport()                                      = 0;
    virtual void         Resize()                                             = 0;
    virtual float        GetAspect()                                          = 0;
    virtual void         Begin()                                              = 0;
    virtual void         Submit()                                             = 0;
    virtual void         Present()                                            = 0;
    virtual void         UpdateUniforms(const std::shared_ptr<Camera> camera) = 0;
    virtual RendererType Type() const                                         = 0;
    virtual void         WaitForIdle()                                        = 0;
    virtual void*        GetCommandBuffer()                                   = 0;
    virtual void         BindPipeline(RenderPipeline pipe)                    = 0;

    virtual void CreateBuffer(
        std::shared_ptr<Buffer>& buffer,
        BufferType               bufferType,
        void*                    data,
        uint32_t                 elementSize,
        uint32_t                 elementCount
    ) = 0;

    void DrawWithBuffers(std::shared_ptr<Buffer> vertexBuffer, std::shared_ptr<Buffer> indexBuffer)
    {
        auto commandBuffer = GetCommandBuffer();
        if (commandBuffer != nullptr)
        {
            vertexBuffer->Bind(commandBuffer);
            indexBuffer->Bind(commandBuffer);
            indexBuffer->Draw(commandBuffer);
            m_frameBuffers.push_back(vertexBuffer);
            m_frameBuffers.push_back(indexBuffer);
        }
    }

    // Hold so we don't call Buffer destructor
    // while still in use by command buffer.
    std::vector<std::shared_ptr<Buffer>> m_frameBuffers;
};
} // namespace drive
