#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <PerlinNoise.hpp>

#include "../Renderer/Renderer.h"
#include "Chunk.h"

#define TERRAIN_DISTANCE         4
#define CHUNK_ARR_SIZE           (2 * TERRAIN_DISTANCE + 1)
#define TERRAIN_CHUNK_RESOLUTION 2
#define TERRAIN_NOISE_SCALE      0.0025f
#define TERRAIN_HEIGHT           100
#define ROAD_NOISE_THRESHOLD     0.9f
#define ROAD_HEIGHT              0.25f

namespace drive
{
class Terrain
{
  public:
    Terrain() = delete;
    Terrain(std::shared_ptr<Renderer> renderer);
    ~Terrain();

    Terrain(const Terrain&)            = delete;
    Terrain(Terrain&&)                 = delete;
    Terrain& operator=(const Terrain&) = delete;
    Terrain& operator=(Terrain&&)      = delete;

    void SetObserverPosition(glm::vec3 pos);

    void Render();

  private:
    void MoveChunks(glm::ivec2 delta);
    void LoadChunks();
    void GenerateChunk(std::shared_ptr<Chunk> chunk);

    Vertex_P_N_C GenerateTerrain(glm::vec2 worldPos);
    float        TerrainHeight(glm::vec2 pos);
    float        TerrainNoise(glm::vec2 pos, int octaves);
    float        RoadNoise(glm::vec2 pos);

    // Returns a loaded chunk at position (chunk-space).
    // nullptr if position is not loaded.
    std::shared_ptr<Chunk> GetChunkAt(glm::ivec2 position)
    {
        auto x = position.x % CHUNK_ARR_SIZE;
        auto y = position.y % CHUNK_ARR_SIZE;
        if (x < 0)
        {
            x += CHUNK_ARR_SIZE;
        }
        if (y < 0)
        {
            y += CHUNK_ARR_SIZE;
        }
        if (m_loadedChunks[x][y]->position == position)
        {
            return m_loadedChunks[x][y];
        }
        return nullptr;
    }

    std::shared_ptr<Chunk> m_loadedChunks[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];

    glm::ivec2 m_observerPosition;

    std::shared_ptr<Renderer> m_renderer;

    siv::PerlinNoise::seed_type  m_perlinSeed;
    siv::BasicPerlinNoise<float> m_perlin;
};
}; // namespace drive
