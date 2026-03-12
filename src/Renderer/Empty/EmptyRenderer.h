#include "../Renderer.h"

namespace drive
{
class EmptyRenderer final : public Renderer
{
  public:
    EmptyRenderer() {};
    ~EmptyRenderer() {};

    EmptyRenderer(const EmptyRenderer&)            = delete;
    EmptyRenderer(EmptyRenderer&&)                 = delete;
    EmptyRenderer& operator=(const EmptyRenderer&) = delete;
    EmptyRenderer& operator=(EmptyRenderer&&)      = delete;

    void SetWindow(std::shared_ptr<Window> /*window*/) override
    {
    }

    void ResetViewport() override
    {
    }

    void SetViewport(Rect /*rect*/) override
    {
    }

    void ClearViewport() override
    {
    }

    void Resize() override
    {
    }

    float GetAspect() override
    {
        return 0.0f;
    }

    void Begin() override
    {
    }

    void Submit() override
    {
    }

    void Present() override
    {
    }

    void UpdateUniforms(const std::shared_ptr<Camera> /*camera*/) override
    {
    }

    RendererType Type() const override
    {
        return RendererType::EMPTY;
    }

    void WaitForIdle() override
    {
    }

    void* GetCommandBuffer() override
    {
        return nullptr;
    }

    void BindPipeline(RenderPipeline /*pipe*/) override
    {
    }

    void CreateBuffer(
        std::shared_ptr<Buffer>& /*buffer*/,
        BufferType /*bufferType*/,
        void* /*data*/,
        uint32_t /*elementSize*/,
        uint32_t /*elementCount*/
    ) override
    {
    }
};
} // namespace drive
