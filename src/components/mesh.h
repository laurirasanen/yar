#pragma once

#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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
};

struct VertexShaded

{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

typedef uint32_t Index;

class Mesh
{
  public:
    Mesh() = delete;
    Mesh(std::string path);
    ~Mesh();

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;

  private:
    std::string m_path;

    VertexType m_vertexType;

    void*              m_vertices;
    std::vector<Index> m_indices;

    glm::vec3 m_min;
    glm::vec3 m_max;
};
}; // namespace yar
