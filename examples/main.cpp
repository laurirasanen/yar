#include "camera.h"
#include "entry.h"
#include "iapp.h"
#include "iassets.h"
#include "iengine.h"
#include "input.h"
#include "irenderer.h"
#include "iui.h"
#include "iwindow.h"
#include "iworld.h"
#include "log.h"
#include "transform.h"

#include <array>

using namespace yar;

class ExampleApp : public IApplication
{
  public:
    int Start() override
    {
        LOG_INFO("Hello from example");

        g_window->SetTitle("example");

        m_camera = std::make_shared<NoclipCamera>();
        m_camera->transform.SetPosition({0.0f, 0.15f, -0.6f});
        m_camera->transform.SetEulerRotation({-10.0f, 0.0f, 0.0f});

        g_renderer->SetCamera(m_camera);

        m_loaded = false;
        return 0;
    }

    int Frame() override
    {
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

        if (m_flightHelmet == nullptr || m_damagedHelmet == nullptr)
        {
            return 0;
        }

        const std::array<std::shared_ptr<INode>, 2> nodes = {m_flightHelmet, m_damagedHelmet};
        const float rotateSpeeds[]                        = {12.5f, -12.5f, 25.0f, -25.0f, 50.0f};
        for (size_t i = 0; i < nodes.size(); i++)
        {
            const float delta =
                static_cast<float>(Time::DeltaFrame) * rotateSpeeds[i % ARRAY_SIZE(rotateSpeeds)];
            const auto& node = nodes[i];
            Transform   t    = node->GetTransform();
            t.AddRotation(delta, VEC_UP);
            node->SetTransform(t);
        }

        return 0;
    }

    int Tick() override
    {
        if (!m_loaded)
        {
            g_ui->ShowLoadingScreen();
            Load();
            g_ui->HideLoadingScreen();
            g_world->SetEnabled(true);
            m_loaded = true;
        }

        return 0;
    }

    void Load()
    {
        Transform trans = {};

        m_flightHelmet = g_assets->LoadGLTF("assets/scenes/FlightHelmet.glb");
        trans.SetEulerRotation({0, 0, 0});
        trans.SetPosition({-0.4, -0.3, 0});
        m_flightHelmet->SetTransform(trans);
        g_world->AddNode(m_flightHelmet);

        m_damagedHelmet = g_assets->LoadGLTF("assets/scenes/DamagedHelmet.glb");
        trans           = {};
        trans.SetEulerRotation({90, 0, 0});
        trans.SetScale({0.3, 0.3, 0.3});
        trans.SetPosition({0.4, 0, 0});
        m_damagedHelmet->SetTransform(trans);
        g_world->AddNode(m_damagedHelmet);

        auto sky = g_assets->LoadSky("assets/ibl/cobble");
        g_world->SetSky(sky);
        g_renderer->SetIBLStrength(1.0f);
    }

  private:
    bool m_loaded;

    std::shared_ptr<INode> m_flightHelmet;
    std::shared_ptr<INode> m_damagedHelmet;

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
