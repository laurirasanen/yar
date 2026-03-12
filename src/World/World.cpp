#include "World.h"
#include "../Log.h"
#include "src/Renderer/Renderer.h"

namespace yar
{

World::World(std::shared_ptr<Renderer> renderer) : m_renderer(renderer)
{
    LOG_DEBUG("Creating World");
    m_sky     = std::make_unique<Sky>(renderer);

    // test plane
    std::vector<Vertex_P_C> planeVertices = {
        {{-0.5, -0.5, 100.0}, {1.0f, 0.0f, -0.1f}},
        { {0.5, -0.5, 100.0}, {0.0f, 1.0f, -0.1f}},
        {  {0.5, 0.5, 100.0},  {0.0f, 0.0f, 0.9f}},
        { {-0.5, 0.5, 100.0},  {1.0f, 1.0f, 0.9f}},
    };
    std::vector<Index> planeIndices = {0, 1, 2, 2, 3, 0};
    renderer->CreateBuffer(
        m_testPlaneVertexBuffer,
        VertexBuffer,
        planeVertices.data(),
        sizeof(Vertex_P_C),
        static_cast<uint32_t>(planeVertices.size())
    );
    renderer->CreateBuffer(
        m_testPlaneIndexBuffer,
        IndexBuffer,
        planeIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(planeIndices.size())
    );
}

World::~World()
{
    LOG_DEBUG("Destroying World");
}

void World::Frame()
{
}

void World::Tick(std::shared_ptr<Camera> camera)
{
    {
        std::scoped_lock worldLock {m_worldMutex};
    }
}

void World::Render()
{
    m_renderer->BindPipeline(RenderPipeline::TEST);
    m_renderer->DrawWithBuffers(m_testPlaneVertexBuffer, m_testPlaneIndexBuffer);

    m_sky->Render();
}
} // namespace yar
