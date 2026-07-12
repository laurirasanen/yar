#pragma once

#include <memory>
#include <mutex>

#include "../public/ecs/entity.h"
#include "../public/ecs/sky.h"
#include "../public/world/iworld.h"

namespace yar
{
class World : public IWorld
{
  public:
    World();
    ~World();

    World(const World&)            = delete;
    World(World&&)                 = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&)      = delete;

    void AddEntity(std::shared_ptr<Entity> entity) override;

    void Update(float deltaTime) override;
    void FixedUpdate(float deltaTime) override;
    void Render() override;

  private:
    std::mutex m_worldMutex;

    std::vector<std::shared_ptr<Entity>> m_entities;

    SkyComponent* m_sky;
};
} // namespace yar
