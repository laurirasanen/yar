#include <array>

#include <glm/geometric.hpp>

#include "../Log.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Vulkan/VulkanRenderer.h"
#include "Terrain.h"

namespace drive
{

Terrain::Terrain(std::shared_ptr<Renderer> renderer) :
    m_renderer(renderer),
    m_perlinSeed(0xDEADBEEF),
    m_perlin(m_perlinSeed)
{
    LOG_DEBUG("Creating Terrain");
    m_observerPosition = {};
    LoadChunks();
}

Terrain::~Terrain()
{
    LOG_DEBUG("Destroying Terrain");
}

void Terrain::SetObserverPosition(glm::vec3 pos)
{
    auto chunkPos = Chunk::WorldToChunk(glm::vec2(pos.x, pos.y));

    if (m_observerPosition == chunkPos)
    {
        return;
    }

    auto delta         = m_observerPosition - chunkPos;
    m_observerPosition = chunkPos;

    MoveChunks(delta);
    LoadChunks();
}

void Terrain::Render()
{
    for (int x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            auto chunk = m_loadedChunks[x][y];

            if (chunk != nullptr)
            {
                if (chunk->vertexBuffer == nullptr)
                {
                    m_renderer->CreateBuffer(
                        chunk->vertexBuffer,
                        VertexBuffer,
                        chunk->vertices.data(),
                        sizeof(Vertex_P_N_C),
                        static_cast<uint32_t>(chunk->vertices.size())
                    );
                    m_renderer->CreateBuffer(
                        chunk->indexBuffer,
                        IndexBuffer,
                        chunk->indices.data(),
                        sizeof(Index),
                        static_cast<uint32_t>(chunk->indices.size())
                    );
                    chunk->vertices.clear();
                    chunk->indices.clear();
                }

                if (chunk->vertexBuffer && chunk->indexBuffer)
                {
                    m_renderer->BindPipeline(RenderPipeline::TERRAIN);
                    m_renderer->DrawWithBuffers(chunk->vertexBuffer, chunk->indexBuffer);
                }
            }
        }
    }
}

void Terrain::MoveChunks(glm::ivec2 delta)
{
    if (delta.x >= CHUNK_ARR_SIZE || delta.x <= -CHUNK_ARR_SIZE || delta.y >= CHUNK_ARR_SIZE
        || delta.y <= -CHUNK_ARR_SIZE)
    {
        for (int x = 0; x < CHUNK_ARR_SIZE; x++)
        {
            for (int y = 0; y < CHUNK_ARR_SIZE; y++)
            {
                m_loadedChunks[x][y].reset();
            }
        }
        return;
    }

    if (delta.y < 0)
    {
        for (int x = 0; x < CHUNK_ARR_SIZE; x++)
        {
            for (int y = -delta.y; y < CHUNK_ARR_SIZE; y++)
            {
                m_loadedChunks[x][y + delta.y] = std::move(m_loadedChunks[x][y]);
            }
        }
    }
    else if (delta.y > 0)
    {
        for (int x = 0; x < CHUNK_ARR_SIZE; x++)
        {
            for (int y = CHUNK_ARR_SIZE - 1; y >= delta.y; y--)
            {
                m_loadedChunks[x][y] = std::move(m_loadedChunks[x][y - delta.y]);
            }
        }
    }

    if (delta.x < 0)
    {
        for (int x = -delta.x; x < CHUNK_ARR_SIZE; x++)
        {
            for (int y = 0; y < CHUNK_ARR_SIZE; y++)
            {
                m_loadedChunks[x + delta.x][y] = std::move(m_loadedChunks[x][y]);
            }
        }
    }
    else if (delta.x > 0)
    {
        for (int x = CHUNK_ARR_SIZE - 1; x >= delta.x; x--)
        {
            for (int y = 0; y < CHUNK_ARR_SIZE; y++)
            {
                m_loadedChunks[x][y] = std::move(m_loadedChunks[x - delta.x][y]);
            }
        }
    }
}

// TODO: thread pool
void Terrain::LoadChunks()
{
    for (int x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            if (m_loadedChunks[x][y] == nullptr)
            {
                auto chunk = std::make_shared<Chunk>(
                    glm::ivec2(x, y) + m_observerPosition - glm::ivec2(TERRAIN_DISTANCE)
                );
                GenerateChunk(chunk);
                m_loadedChunks[x][y] = chunk;
            }
        }
    }
}

void Terrain::GenerateChunk(std::shared_ptr<Chunk> chunk)
{
    const unsigned int quadsPerSide    = (CHUNK_SIZE * TERRAIN_CHUNK_RESOLUTION);
    const unsigned int verticesPerSide = quadsPerSide + 1;
    const unsigned int verticesCount   = verticesPerSide * verticesPerSide;
    const unsigned int indicesCount    = quadsPerSide * quadsPerSide * 6;

    chunk->vertices = std::vector<Vertex_P_N_C>(verticesCount);
    chunk->indices  = std::vector<Index>(indicesCount);

    for (unsigned int x = 0; x < verticesPerSide; x++)
    {
        const float xOffset = static_cast<float>(x) / TERRAIN_CHUNK_RESOLUTION;

        for (unsigned int y = 0; y < verticesPerSide; y++)
        {
            const float     yOffset     = static_cast<float>(y) / TERRAIN_CHUNK_RESOLUTION;
            const glm::vec2 vertexWorld = chunk->worldPosition + glm::vec2(xOffset, yOffset);

            chunk->vertices[x * verticesPerSide + y] = GenerateTerrain(vertexWorld);
        }
    }

    for (unsigned int x = 0; x < quadsPerSide; x++)
    {
        for (unsigned int y = 0; y < quadsPerSide; y++)
        {
            const unsigned int firstIndex  = (x * quadsPerSide + y) * 6;
            const unsigned int firstVertex = x * verticesPerSide + y;

            chunk->indices[firstIndex + 0] = {firstVertex + 0};
            chunk->indices[firstIndex + 1] = {firstVertex + verticesPerSide};
            chunk->indices[firstIndex + 2] = {firstVertex + 1};

            chunk->indices[firstIndex + 3] = {firstVertex + 1};
            chunk->indices[firstIndex + 4] = {firstVertex + verticesPerSide};
            chunk->indices[firstIndex + 5] = {firstVertex + verticesPerSide + 1};
        }
    }
}

Vertex_P_N_C Terrain::GenerateTerrain(glm::vec2 worldPos)
{
    const auto  noisePos     = worldPos * TERRAIN_NOISE_SCALE;
    const float vertexHeight = TerrainHeight(noisePos);
    const auto  pos          = glm::vec3(worldPos.x, worldPos.y, vertexHeight);

    // Figure out the vertex normal by sampling noise from 2 more spots
    const float noiseNormalOffset = 0.5f * TERRAIN_NOISE_SCALE;
    const auto  xPos              = glm::vec2(noisePos.x + noiseNormalOffset, noisePos.y);
    const auto  yPos              = glm::vec2(noisePos.x, noisePos.y + noiseNormalOffset);
    const float heightX           = TerrainHeight(xPos);
    const float heightY           = TerrainHeight(yPos);
    const auto  vX = glm::vec3(xPos.x / TERRAIN_NOISE_SCALE, xPos.y / TERRAIN_NOISE_SCALE, heightX);
    const auto  vY = glm::vec3(yPos.x / TERRAIN_NOISE_SCALE, yPos.y / TERRAIN_NOISE_SCALE, heightY);
    const auto  tangent   = vX - pos;
    const auto  bitangent = vY - pos;
    const auto  normal    = glm::normalize(glm::cross(tangent, bitangent));

    const auto grassColor    = glm::vec3(0.0f, 0.2f, 0.0f);
    const auto roadColor     = glm::vec3(0.1f, 0.1f, 0.1f);
    const auto roadSideColor = glm::vec3(0.2f, 0.10f, 0.075f);

    const auto road = RoadNoise(noisePos);

    auto color = grassColor;
    if (road > ROAD_NOISE_THRESHOLD)
    {
        color = roadColor;
    }
    else if (road > 0)
    {
        color = roadSideColor;
    }

    return Vertex_P_N_C {
        {worldPos.x, worldPos.y, vertexHeight},
        normal,
        color
    };
}

float Terrain::TerrainHeight(glm::vec2 pos)
{
    const float terrain = TerrainNoise(pos, 6);

    const float roadNoise   = RoadNoise(pos);
    const float roadTerrain = TerrainNoise(pos, 3);

    const float smoothTerrain = std::lerp(terrain, roadTerrain, roadNoise);

    float roadHeight = 0.0f;
    if (roadNoise > ROAD_NOISE_THRESHOLD)
    {
        roadHeight = ROAD_HEIGHT * roadNoise;
    }

    return smoothTerrain * TERRAIN_HEIGHT + roadHeight;
}

float Terrain::TerrainNoise(glm::vec2 pos, int octaves)
{
    return m_perlin.octave2D_01(pos.x, pos.y, octaves);
}

float Terrain::RoadNoise(glm::vec2 pos)
{
    const auto roadX          = sin(pos.y * 0.5f) * 1.0f + cos(pos.y * 1.3f) * 0.3f;
    const auto roadHalfWidth  = 2.5f;
    const auto smoothDistance = 5.0f;
    float      xDist          = abs(roadX - pos.x) / TERRAIN_NOISE_SCALE;
    if (xDist > roadHalfWidth)
    {
        if (xDist > smoothDistance)
        {

            return 0;
        }
        return std::lerp(ROAD_NOISE_THRESHOLD, 0.2f, (xDist - roadHalfWidth) / smoothDistance);
    }
    return std::lerp(1.0f, ROAD_NOISE_THRESHOLD, xDist / roadHalfWidth);
}
}; // namespace drive
