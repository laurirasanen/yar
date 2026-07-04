#pragma once

#include "common.h"
#include "data_types.h"
#include "pipeline.h"

#include <memory>

namespace yar
{
class PostProcessPass
{
  public:
    PostProcessPass() = delete;

    virtual ~PostProcessPass();

    PostProcessPass(const PostProcessPass&)            = delete;
    PostProcessPass(PostProcessPass&&)                 = delete;
    PostProcessPass& operator=(const PostProcessPass&) = delete;
    PostProcessPass& operator=(PostProcessPass&&)      = delete;

    PostProcessPass(const char* shader, VkFormat outputColorFormat);

    virtual void SetInputs(
        const RenderAttachment& colorInput,
        const RenderAttachment& depthInput,
        uint32_t                frameIndex
    );

    void SetOutput(const VkImage output);

    void CreateOutput();

    void ResizeOutput();

    virtual void PreRender()
    {
    }

    virtual void Render(uint32_t frameIndex) = 0;

    virtual void PostRender();

    const RenderAttachment& GetOutput() const
    {
        return m_colorOutput;
    }

  protected:
    void DestroyOutput();

    std::shared_ptr<VulkanPipeline<VertexEmpty>> m_pipeline;
    VkDescriptorSetLayout                        m_descriptorSetLayout;
    std::vector<VkDescriptorSet>                 m_descriptorSets;

    RenderAttachment m_colorInput;
    RenderAttachment m_depthInput;

    RenderAttachment m_colorOutput;
    bool             m_ownOutput;
};

class TonemapPass : public PostProcessPass
{
  public:
    TonemapPass(VkFormat outputColorFormat) : PostProcessPass("tonemap.slang", outputColorFormat)
    {
    }

    ~TonemapPass()
    {
    }

    void Render(uint32_t frameIndex) override;
};
}; // namespace yar
