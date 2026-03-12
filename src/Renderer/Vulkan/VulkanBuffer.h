#pragma once

#include <cstring>
#include <memory>

#include "../Buffer.h"
#include "VmaUsage.h"

namespace drive
{
class VulkanBuffer : public Buffer
{
  public:
    VulkanBuffer() = delete;
    VulkanBuffer(
        BufferType     bufferType,
        BufferLocation bufferLocation,
        uint32_t       elementSize,
        uint32_t       elementCount
    );

    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&)            = delete;
    VulkanBuffer(VulkanBuffer&&)                 = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(VulkanBuffer&&)      = delete;

    void Write(void* data, size_t size) override;

    void CopyToDevice(void* commandBuffer, std::shared_ptr<Buffer> deviceBuffer) override;

    void Clear() override;

    void Bind(void* commandBuffer) override;

    void Draw(void* commandBuffer) override;
    void Draw(void* commandBuffer, uint32_t indexOffset, uint32_t indexCount, int32_t vertexOffset)
        override;

    void Map(void** data) override;
    void Unmap() override;

    VkBuffer GetVkBuffer() const
    {
        return m_vkBuffer;
    }

  private:
    VkBuffer      m_vkBuffer;
    VmaAllocation m_vmaAllocation;
};
} // namespace drive
