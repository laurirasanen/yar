#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../renderer/buffer.h"

namespace yar
{
enum VertexType
{
    Unlit,
    Shaded,
};

struct VertexUnlit
{
    glm::vec3 position;
    glm::vec3 color;
};

struct VertexShaded

{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

typedef uint32_t Index;

template<class T>
class Mesh
{
  public:
    Mesh() = delete;

    Mesh(
        std::vector<T>          vertices,
        std::vector<Index>      indices,
        std::shared_ptr<Buffer> vertexBuffer,
        std::shared_ptr<Buffer> indexBuffer
    ) :
        m_vertices(vertices),
        m_indices(indices),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer)
    {
    }

    ~Mesh()
    {
    }

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;

    const std::vector<T>& GetVertices()
    {
        return m_vertices;
    }

    const std::vector<Index>& GetIndices()
    {
        return m_indices;
    }

    std::shared_ptr<Buffer> GetVertexBuffer()
    {
        return m_vertexBuffer;
    }

    std::shared_ptr<Buffer> GetIndexBuffer()
    {
        return m_indexBuffer;
    }

  private:
    VertexType m_vertexType;

    std::vector<T>     m_vertices;
    std::vector<Index> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;

    glm::vec3 m_min;
    glm::vec3 m_max;
};
}; // namespace yar
