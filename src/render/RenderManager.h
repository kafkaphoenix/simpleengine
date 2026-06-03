#pragma once

#include <glm/mat4x4.hpp>
#include <memory>
#include <optional>
#include <span>

#include "Framebuffer.h"
#include "Frustum.h"
#include "ModelRenderer.h"
#include "PostProcessRenderer.h"
#include "RenderStats.h"
#include "SkyboxRenderer.h"
#include "core/Config.h"

namespace se::assets {
class Cubemap;
}

namespace se::scene {
class Camera;
struct LightData;
struct Renderable;
struct AnimatedRenderable;
}

namespace se::render {

class RenderManager {
public:
    explicit RenderManager(const se::core::config::Render& renderConfig);

    void beginFrame(const se::scene::Camera& camera);
    void submit(const se::scene::Renderable& renderable);
    void endFrame(const se::scene::LightData& lights);

    void resizeFramebuffer(int width, int height);
    void setSkybox(std::shared_ptr<se::assets::Cubemap> cubemap);
    void toggleWireframe();
    void cyclePostEffect();
    void setPostEffect(PostEffect effect);
    void setBatchSize(size_t maxInstances);
    void reset();

    [[nodiscard]] PostEffect getPostEffect() const noexcept { return m_PostProcess.getEffect(); }
    [[nodiscard]] const RenderStats& getStats() const noexcept { return m_Stats; }

private:
    void initFramebuffer(int width, int height);
    void resolveMsaaToFinalFramebuffer() const;
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