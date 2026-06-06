#pragma once

#include <glm/mat4x4.hpp>
#include <optional>
#include <span>

#include "Framebuffer.h"
#include "Frustum.h"
#include "ModelRenderer.h"
#include "PostProcessRenderer.h"
#include "RenderStats.h"
#include "SkyboxRenderer.h"
#include "assets/AssetHandle.h"

namespace se::scene {
class Camera;
struct LightData;
struct Renderable;
struct AnimatedRenderable;
}

namespace se::core {
class Config;
}

namespace se::render {

class RenderManager {
public:
    explicit RenderManager(const se::core::Config& config);

    void beginFrame(const se::scene::Camera& camera);
    void submit(const se::scene::Renderable& renderable);
    void endFrame(const se::scene::LightData& lights);

    void resizeFramebuffer(int width, int height);
    void setSkybox(se::assets::CubemapHandle cubemap);
    void toggleWireframe();
    void cycleRenderDebugView();
    void cyclePostEffect();
    void setPostEffect(PostEffect effect);
    void setBatchSize(size_t maxInstances);
    void reset();

    [[nodiscard]] PostEffect getPostEffect() const noexcept { return m_PostProcess.getEffect(); }
    [[nodiscard]] const RenderStats& getStats() const noexcept { return m_Stats; }

private:
    void initFramebuffer(int width, int height);
    static void clearSceneFramebuffer(const Framebuffer& framebuffer);
    static void clearTransparencyTargets(const Framebuffer& framebuffer);
    void resolveMsaaSceneToFinalFramebuffer() const;
    static void setupGlState();

    const se::scene::Camera* m_Camera = nullptr;
    Frustum m_Frustum{};
    ModelRenderer m_ModelRenderer;
    SkyboxRenderer m_SkyboxRenderer;
    PostProcessRenderer m_PostProcess;
    RenderStats m_Stats;
    bool m_Wireframe = false;
    int m_MsaaSamples = 4;
    std::optional<Framebuffer> m_SceneMsaaFbo;
    std::optional<Framebuffer> m_SceneFinalFbo;
};

}  // namespace se::render