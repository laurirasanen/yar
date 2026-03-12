#include "VulkanBuffer.h"

namespace drive
{
VulkanBuffer::VulkanBuffer(
    BufferType     bufferType,
    BufferLocation bufferLocation,
    uint32_t       elementSize,
    uint32_t       elementCount
) :
    Buffer(bufferType, bufferLocation, elementSize, elementCount)
{
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = m_size;
    bufferInfo.usage       = 0;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo {};

    switch (m_bufferType)
    {
        case VertexBuffer:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        }

        case IndexBuffer:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        }

        case UniformBuffer:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            allocInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        }

        default:
        {
            std::runtime_error("Unhandled buffer type");
        }
    }

    switch (m_bufferLocation)
    {
        case Host:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            allocInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocInfo.preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

            break;
        }

        case Device:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocInfo.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            break;
        }

        default:
        {
            throw std::runtime_error("Unhandled buffer destination");
        }
    }

    vmaCreateBuffer(g_vma, &bufferInfo, &allocInfo, &m_vkBuffer, &m_vmaAllocation, nullptr);
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_isMapped)
    {
        Unmap();
    }
    vmaDestroyBuffer(g_vma, m_vkBuffer, m_vmaAllocation);
}

void VulkanBuffer::Write(void* data, size_t size)
{
    vmaCopyMemoryToAllocation(g_vma, data, m_vmaAllocation, 0, size);
}

void VulkanBuffer::CopyToDevice(void* commandBuffer, std::shared_ptr<Buffer> deviceBuffer)
{
    if (m_bufferLocation != Host)
    {
        throw std::runtime_error("Tried copying from a non-host buffer");
    }

    if (deviceBuffer->GetLocation() != Device)
    {
        throw std::runtime_error("Tried copying to a non-device buffer");
    }

    VkDeviceSize requiredSize = GetElementCount() * GetElementSize();
    if (deviceBuffer->GetSize() < requiredSize)
    {
        throw std::runtime_error("Tried copying to a buffer that is too small");
    }

    auto vkCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer);

    VkBufferCopy copyRegion {};
    copyRegion.size      = requiredSize;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;

    auto vkDeviceBuffer = static_pointer_cast<VulkanBuffer>(deviceBuffer);
    vkCmdCopyBuffer(vkCommandBuffer, m_vkBuffer, vkDeviceBuffer->GetVkBuffer(), 1, &copyRegion);

    deviceBuffer->SetElementCount(m_elementCount);
}

void VulkanBuffer::Clear()
{
    if (m_bufferLocation != Host)
    {
        throw std::runtime_error("Tried mapping a non-host buffer");
    }

    VmaAllocationInfo allocInfo {};
    vmaGetAllocationInfo(g_vma, m_vmaAllocation, &allocInfo);

    void* mappedData;
    vmaMapMemory(g_vma, m_vmaAllocation, &mappedData);
    std::memset(mappedData, 0, allocInfo.size);
    vmaUnmapMemory(g_vma, m_vmaAllocation);

    if (!(allocInfo.memoryType & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        vmaFlushAllocation(g_vma, m_vmaAllocation, 0, allocInfo.size);
    }
}

void VulkanBuffer::Bind(void* commandBuffer)
{
    if (m_bufferLocation != Device)
    {
        throw std::runtime_error("Tried to bind a non-device buffer");
    }

    auto vkCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer);

    switch (m_bufferType)
    {
        case VertexBuffer:
        {
            VkBuffer     buffers[] = {m_vkBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, buffers, offsets);
            break;
        }

        case IndexBuffer:
        {
            vkCmdBindIndexBuffer(vkCommandBuffer, m_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
            break;
        }

        default:
        {
            throw std::runtime_error("Unhandled buffer type");
        }
    }
}

void VulkanBuffer::Draw(void* commandBuffer)
{
    auto vkCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer);

    switch (m_bufferType)
    {
        case VertexBuffer:
        {
            vkCmdDraw(vkCommandBuffer, m_elementCount, 1, 0, 0);
            break;
        }

        case IndexBuffer:
        {
            vkCmdDrawIndexed(vkCommandBuffer, m_elementCount, 1, 0, 0, 0);
            break;
        }

        default:
        {
            throw std::runtime_error("Unhandled buffer type");
        }
    }
}

void VulkanBuffer::Draw(
    void*    commandBuffer,
    uint32_t indexOffset,
    uint32_t indexCount,
    int32_t  vertexOffset
)
{
    auto vkCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer);

    switch (m_bufferType)
    {
        case IndexBuffer:
        {
            vkCmdDrawIndexed(vkCommandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
            break;
        }

        default:
        {
            throw std::runtime_error("Unhandled buffer type");
        }
    }
}

void VulkanBuffer::Map(void** data)
{
    if (m_isMapped)
    {
        throw std::runtime_error("Tried to map a buffer that is already mapped");
    }

    vmaMapMemory(g_vma, m_vmaAllocation, data);
    m_isMapped = true;
}

void VulkanBuffer::Unmap()
{
    if (!m_isMapped)
    {
        throw std::runtime_error("Tried to unmap buffer that isn't mapped");
    }

    vmaUnmapMemory(g_vma, m_vmaAllocation);
    m_isMapped = false;
}
}; // namespace drive
