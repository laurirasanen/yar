#pragma once

#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>

namespace drive
{

enum BufferType
{
    VertexBuffer,
    IndexBuffer,
    UniformBuffer,
};

enum BufferLocation
{
    Host,
    Device,
};

class Buffer
{
  public:
    Buffer() = delete;

    Buffer(
        BufferType     bufferType,
        BufferLocation bufferLocation,
        uint32_t       elementSize,
        uint32_t       elementCount
    ) :
        m_bufferType(bufferType),
        m_bufferLocation(bufferLocation),
        m_elementSize(elementSize),
        m_elementCount(elementCount),
        m_size(m_elementSize * m_elementCount)
    {
    }

    virtual ~Buffer() {};

    Buffer(const Buffer&)            = delete;
    Buffer(Buffer&&)                 = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&)      = delete;

    template<class IT>
    requires std::contiguous_iterator<IT>
    void Write(const IT it, uint32_t length)
    {
        if (m_bufferLocation != Host)
        {
            throw std::runtime_error("Tried mapping a non-host buffer");
        }

        size_t size = length * m_elementSize;
        if (size > m_size)
        {
            throw std::runtime_error("Tried to write more data than allocated");
        }

        Write(static_cast<void*>(it), size);
        m_elementCount = length;
    }

    virtual void Write(void* data, size_t size) = 0;

    virtual void CopyToDevice(void* commandBuffer, std::shared_ptr<Buffer> deviceBuffer) = 0;

    virtual void Clear() = 0;

    virtual void Bind(void* commandBuffer) = 0;

    virtual void Draw(void* commandBuffer) = 0;
    virtual void Draw(
        void*    commandBuffer,
        uint32_t indexOffset,
        uint32_t indexCount,
        int32_t  vertexOffset
    ) = 0;

    virtual void Map(void** data) = 0;
    virtual void Unmap()          = 0;

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

  protected:
    BufferType     m_bufferType;
    BufferLocation m_bufferLocation;
    uint32_t       m_elementSize;
    uint32_t       m_elementCount;
    size_t         m_size;
    bool           m_isMapped = false;
};
} // namespace drive
