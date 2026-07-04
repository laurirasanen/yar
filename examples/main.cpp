#include "camera.h"
#include "entry.h"
#include "iapp.h"
#include "iassets.h"
#include "iengine.h"
#include "input.h"
#include "iphysics.h"
#include "irenderer.h"
#include "iui.h"
#include "iwindow.h"
#include "iworld.h"
#include "log.h"
#include "time_util.h"
#include "transform.h"

using namespace yar;

class ExampleApp : public IApplication
{
  public:
    int Start() override
    {
        LOG_INFO("Hello from example");

        g_window->SetTitle("example");

        m_camera = std::make_shared<NoclipCamera>();
        m_camera->transform.SetPosition({0.10f, 0.15f, -0.8f});
        m_camera->Pitch = 10.0f;

        g_renderer->SetCamera(m_camera);

        m_loaded = false;
        return 0;
    }

    void Frame() override
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

        const float delta = static_cast<float>(Time::DeltaFrame) * 10.0f;
        Transform   t     = m_flightHelmet->GetTransform();
        t.AddRotation(delta, VEC_UP);
        m_flightHelmet->SetTransform(t, true);
    }

    void Tick() override
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
        Transform trans = {};

        m_flightHelmet = g_assets->LoadGLTF("assets/scenes/FlightHelmet.glb");
        trans.SetEulerRotation({0, 0, 0});
        trans.SetPosition({-0.4f, -0.3f, 0});
        m_flightHelmet->SetTransform(trans, true);
        g_world->AddNode(m_flightHelmet);

        m_damagedHelmet = g_world->AddPhysicsNode(
            "helmet",
            PhysicsBodyType::BODY_DYNAMIC,
            PhysicsBodyShape::SHAPE_SPHERE,
            {0, 2.0f, 0},
            glm::identity<glm::quat>(),
            {0.2f, 0.2f, 0.2f}
        );
        auto meshNode = g_assets->LoadGLTF("assets/scenes/DamagedHelmet.glb");
        trans         = {};
        trans.SetEulerRotation({90.0f, 180.0f, 0});
        trans.SetScale({0.25f, 0.25f, 0.25f});
        meshNode->SetTransform(trans, true);
        m_damagedHelmet->AddChild(meshNode);

        auto sky = g_assets->LoadSky("assets/ibl/cobble");
        g_world->SetSky(sky);
        g_renderer->SetIBLStrength(1.0f);
        g_renderer->SetExposure(1.0f);
        g_renderer->SetContrast(1.0f);

        m_floor = g_world->AddPhysicsNode(
            "floor",
            PhysicsBodyType::BODY_STATIC,
            PhysicsBodyShape::SHAPE_BOX,
            {0, -1.0f, 0},
            glm::angleAxis(-glm::radians(5.0f), VEC_Z),
            {5.0f, 0.1f, 5.0f}
        );
        auto floorMesh = g_assets->CreateBox({5.0f, 0.1f, 5.0f}, {0.5f, 0.5f, 0.5f});
        m_floor->AddChild(floorMesh);

        m_wall = g_world->AddPhysicsNode(
            "wall",
            PhysicsBodyType::BODY_STATIC,
            PhysicsBodyShape::SHAPE_BOX,
            {5.0f, 0, 0},
            glm::identity<glm::quat>(),
            {0.1f, 5.0f, 5.0f}
        );
        auto wallMesh = g_assets->CreateBox({0.1f, 5.0f, 5.0f}, {0.8f, 0.0f, 0.0f});
        m_wall->AddChild(wallMesh);
    }

  private:
    bool m_loaded;

    std::shared_ptr<INode> m_flightHelmet;
    std::shared_ptr<INode> m_damagedHelmet;
    std::shared_ptr<INode> m_floor;
    std::shared_ptr<INode> m_wall;

    std::shared_ptr<Camera> m_camera;
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
