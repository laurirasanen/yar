#include "../public/ui/iui.h"
#include "ecs/boxmesh.h"
#include "ecs/camera.h"
#include "ecs/entity.h"
#include "ecs/rigidbody.h"
#include "ecs/sky.h"
#include "engine/entry.h"
#include "engine/iapp.h"
#include "engine/iengine.h"
#include "engine/iphysics.h"
#include "log.h"
#include "renderer/irenderer.h"
#include "time_util.h"
#include "transform.h"
#include "window/input.h"
#include "window/iwindow.h"
#include "world/iworld.h"

using namespace yar;

class ExampleApp : public IApplication
{
  public:
    int Start() override
    {
        LOG_INFO("Hello from example");

        g_window->SetTitle("example");

        auto cam = std::make_shared<Entity>("camera");
        cam->AddComponent<TransformComponent>();
        cam->AddComponent<NoclipCamera>()->Pitch = 10.0f;
        cam->GetComponent<TransformComponent>()->GetTransform()->SetPosition({0.10f, 0.15f, -0.8f});

        g_world->AddEntity(cam);

        m_loaded = false;
        return 0;
    }

    void Update(float deltaTime) override
    {
        if (!m_loaded)
        {
            return;
        }

        const auto frameInput = g_engine->GetFrameInput();
        if (frameInput.WasPressed(Key::KEY_EXPOSURE_UP))
        {
            g_renderer->SetExposure(g_renderer->GetExposure() + 0.05f);
        }
        else if (frameInput.WasPressed(Key::KEY_EXPOSURE_DOWN))
        {
            g_renderer->SetExposure(g_renderer->GetExposure() - 0.05f);
        }

        if (frameInput.WasPressed(Key::KEY_CONTRAST_UP))
        {
            g_renderer->SetContrast(g_renderer->GetContrast() + 0.05f);
        }
        else if (frameInput.WasPressed(Key::KEY_CONTRAST_DOWN))
        {
            g_renderer->SetContrast(g_renderer->GetContrast() - 0.05f);
        }

        if (frameInput.WasPressed(Key::KEY_IBL_UP))
        {
            g_renderer->SetIBLStrength(g_renderer->GetIBLStrength() + 0.05f);
        }
        else if (frameInput.WasPressed(Key::KEY_IBL_DOWN))
        {
            g_renderer->SetIBLStrength(g_renderer->GetIBLStrength() - 0.05f);
        }

        Transform* t = m_flightHelmet->GetComponent<TransformComponent>()->GetTransform();
        t->AddRotation(deltaTime * 10.0f, VEC_UP);
    }

    void FixedUpdate(float deltaTime) override
    {
        if (!m_loaded)
        {
            g_ui->ShowLoadingScreen();
            Load();
            g_ui->HideLoadingScreen();
            g_world->SetEnabled(true);
            m_loaded = true;
        }
    }

    void Load()
    {
        auto mesh      = g_resources->Load<Mesh>("assets/scenes/FlightHelmet.glb");
        m_flightHelmet = std::make_shared<Entity>("flight helmet");
        auto trans     = m_flightHelmet->AddComponent<TransformComponent>()->GetTransform();
        m_flightHelmet->AddComponent<MeshComponent>(mesh);
        trans->SetEulerRotation({0, 0, 0});
        trans->SetPosition({-0.4f, -0.3f, 0});

        g_world->AddEntity(m_flightHelmet);

        mesh               = g_resources->Load<Mesh>("assets/scenes/DamagedHelmet.glb");
        auto damagedHelmet = std::make_shared<Entity>("damaged helmet");
        trans              = damagedHelmet->AddComponent<TransformComponent>()->GetTransform();
        trans->SetPosition({0, 2.0f, 0});
        trans->SetEulerRotation({90.0f, 180.0f, 0});
        trans->SetScale({0.25f, 0.25f, 0.25f});
        damagedHelmet->AddComponent<MeshComponent>(mesh);
        auto body = damagedHelmet->AddComponent<RigidBody>(PhysicsBodyType::BODY_DYNAMIC);
        body->AddCollider(PhysicsShapeType::SHAPE_SPHERE, {}, {}, {0.2f, 0.2f, 0.2f});

        g_world->AddEntity(damagedHelmet);

        auto sky = std::make_shared<Entity>("sky");
        sky->AddComponent<SkyComponent>("assets/ibl/cobble");

        g_world->AddEntity(sky);

        g_renderer->SetIBLStrength(0.5f);
        g_renderer->SetExposure(1.0f);
        g_renderer->SetContrast(1.0f);

        auto floor = std::make_shared<Entity>("floor");
        trans      = floor->AddComponent<TransformComponent>()->GetTransform();
        trans->SetPosition({0, -1.0f, 0});
        trans->SetRotation(glm::angleAxis(-glm::radians(5.0f), VEC_Z));
        floor->AddComponent<BoxMeshComponent>({5.0f, 0.1f, 5.0f});
        body = floor->AddComponent<RigidBodyComponent>(PhysicsBodyType::BODY_STATIC);
        body->AddCollider(PhysicsShapeType::SHAPE_BOX, {}, {}, {5.0f, 0.1f, 5.0f});

        g_world->AddEntity(floor);

        auto wall = std::make_shared<Entity>("wall");
        trans     = wall->AddComponent<TransformComponent>()->GetTransform();
        trans->SetPosition({5.0f, 0, 0});
        wall->AddComponent<BoxMeshComponent>({0.1f, 5.0f, 5.0f});
        body = wall->AddComponent<RigidBodyComponent>(PhysicsBodyType::BODY_STATIC);
        body->AddCollider(PhysicsShapeType::SHAPE_BOX, {}, {}, {0.1f, 5.0f, 5.0f});

        g_world->AddEntity(wall);
    }

  private:
    bool m_loaded;

    std::shared_ptr<Entity> m_flightHelmet;
};

int main(int argc, char** argv)
{
    if (!YAR_Init(argc, argv))
    {
        return 1;
    }

    auto app = std::make_shared<ExampleApp>();

    return YAR_Run(app);
}
