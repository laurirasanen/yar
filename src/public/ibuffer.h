#pragma once

#include <cstddef>
#include <cstdint>

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

class IBuffer
{
  public:
    IBuffer(
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

    virtual ~IBuffer() = default;

    IBuffer(const char*) {};

    IBuffer(const IBuffer&)            = delete;
    IBuffer(IBuffer&&)                 = delete;
    IBuffer& operator=(const IBuffer&) = delete;
    IBuffer& operator=(IBuffer&&)      = delete;

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
};
}; // namespace yar
