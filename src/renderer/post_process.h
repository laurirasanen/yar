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

    PostProcessPass(
        const char* name,
        const char* shader,
        uint32_t    numTextures,
        VkFormat    outputColorFormat
    );

    virtual void SetInputs(std::vector<RenderAttachment> inputs, uint32_t frameIndex);

    void SetOutput(const RenderAttachment& output);

    void CreateOutput(uint32_t outputWidth, uint32_t outputHeight);

    virtual void Render(VkCommandBuffer commandBuffer, uint32_t frameIndex);

    virtual void TransitionOutput();

    const RenderAttachment& GetOutput() const
    {
        return m_colorOutput;
    }

    void SetLoadOp(VkAttachmentLoadOp op)
    {
        m_outputLoadOp = op;
    }

    void SetOutputSamplerMode(VkSamplerAddressMode mode)
    {
        m_outputSamplerMode = mode;
    }

  protected:
    void DestroyOutput();

    const char* m_name;

    std::shared_ptr<VulkanPipeline<VertexEmpty>> m_pipeline;
    VkDescriptorSetLayout                        m_descriptorSetLayout;
    std::vector<VkDescriptorSet>                 m_descriptorSets;

    VkAttachmentLoadOp   m_outputLoadOp;
    RenderAttachment     m_colorOutput;
    VkSamplerAddressMode m_outputSamplerMode;
    bool                 m_ownOutput;
};

class DownsamplePass : public PostProcessPass
{
  public:
    DownsamplePass(VkFormat outputColorFormat) :
        PostProcessPass("Downsample", "downsample.slang", 1, outputColorFormat)
    {
    }

    ~DownsamplePass()
    {
    }
};

class UpsamplePass : public PostProcessPass
{
  public:
    UpsamplePass(VkFormat outputColorFormat) :
        PostProcessPass("Upsample", "upsample.slang", 2, outputColorFormat)
    {
    }

    ~UpsamplePass()
    {
    }
};

class TonemapPass : public PostProcessPass
{
  public:
    TonemapPass(VkFormat outputColorFormat) :
        PostProcessPass("Tonemap", "tonemap.slang", 2, outputColorFormat)
    {
    }

    ~TonemapPass()
    {
    }
};
}; // namespace yar
