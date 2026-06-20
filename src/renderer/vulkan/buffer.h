#pragma once

#include <cstring>
#include <memory>

#include "../buffer.h"
#include "vma.h"

namespace yar
{
class VulkanBuffer : public Buffer
{
  public:
    VulkanBuffer() = delete;
    VulkanBuffer(
        VkDevice       device,
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

    void Draw(void* commandBuffer, uint32_t firstInstance, uint32_t instanceCount) override;
    void Draw(
        void*    commandBuffer,
        uint32_t indexOffset,
        uint32_t indexCount,
        int32_t  vertexOffset
    ) override;

    void Map(void** data) override;
    void Unmap() override;

    VkBuffer GetVkBuffer() const
    {
        return m_vkBuffer;
    }

    VkDeviceAddress* GetDeviceAddress()
    {
        if (m_bufferLocation != SecretThirdOption)
        {
            throw std::runtime_error("Unhandled buffer type");
        }
        return &m_vkDeviceAddress;
    }

    VmaAllocationInfo GetAllocationInfo() const
    {
        return m_vmaAllocationInfo;
    }

  private:
    VkBuffer          m_vkBuffer;
    VmaAllocation     m_vmaAllocation;
    VmaAllocationInfo m_vmaAllocationInfo;
    VkDeviceAddress   m_vkDeviceAddress;
};
} // namespace yar
