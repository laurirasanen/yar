#pragma once

#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "vma.h"

namespace yar
{
enum BufferType
{
    VertexBuffer,
    IndexBuffer,
    UniformBuffer,
    ShaderDataBuffer,
    ImageBuffer,
};

enum BufferLocation
{
    Host,
    Device,
    SecretThirdOption,
};

class Buffer
{
  public:
    Buffer() = delete;
    Buffer(
        VkDevice       device,
        BufferType     bufferType,
        BufferLocation bufferLocation,
        uint32_t       elementSize,
        uint32_t       elementCount
    );

    ~Buffer();

    Buffer(const Buffer&)            = delete;
    Buffer(Buffer&&)                 = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&)      = delete;

    template<class IT>
    requires std::contiguous_iterator<IT>
    void Write(const IT it)
    {
        if (m_bufferLocation != Host)
        {
            throw std::runtime_error("Tried mapping a non-host buffer");
        }

        size_t size = it.size() * m_elementSize;
        if (size > m_size)
        {
            throw std::runtime_error("Tried to write more data than allocated");
        }

        Write(static_cast<void*>(it), size);
        m_elementCount = it.size();
    }

    void Write(void* data, size_t size);

    void CopyToDevice(void* commandBuffer, std::shared_ptr<Buffer> deviceBuffer);

    void Clear();

    void Bind(void* commandBuffer);

    void Draw(void* commandBuffer, uint32_t firstInstance, uint32_t instanceCount);
    void Draw(void* commandBuffer, uint32_t indexOffset, uint32_t indexCount, int32_t vertexOffset);

    void Map(void** data);
    void Unmap();

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

    void SetElementCount(uint32_t count)
    {
        m_elementCount = count;
    }

    uint32_t GetElementCount() const
    {
        return m_elementCount;
    }

    constexpr BufferType GetType() const
    {
        return m_bufferType;
    }

    BufferLocation GetLocation() const
    {
        return m_bufferLocation;
    }

    size_t GetSize() const
    {
        return m_size;
    }

    size_t GetElementSize()
    {
        return m_elementSize;
    }

  private:
    VkBuffer          m_vkBuffer;
    VmaAllocation     m_vmaAllocation;
    VmaAllocationInfo m_vmaAllocationInfo;
    VkDeviceAddress   m_vkDeviceAddress;

    BufferType     m_bufferType;
    BufferLocation m_bufferLocation;
    uint32_t       m_elementSize;
    uint32_t       m_elementCount;
    size_t         m_size;
    bool           m_isMapped = false;
};
} // namespace yar
