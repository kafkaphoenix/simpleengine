#pragma once

#include <optional>

#include "Framebuffer.h"
#include "Frustum.h"
#include "ModelRenderer.h"
#include "PostProcessRenderer.h"
#include "RenderStats.h"

namespace se::scene {
class Camera;
struct LightData;
struct Renderable;
}  // namespace se::scene

namespace se::render {

class RenderManager {
public:
    RenderManager();

    void beginFrame(const se::scene::Camera& camera);
    void submit(const se::scene::Renderable& renderable);
    void endFrame(const se::scene::LightData& lights);

    void resizeFramebuffer(int width, int height);
    void toggleWireframe();
    void cyclePostEffect();
    void setPostEffect(PostEffect effect);
    void setBatchSize(size_t maxInstances);
    void reset();

    [[nodiscard]] PostEffect getPostEffect() const noexcept { return m_PostProcess.getEffect(); }
    [[nodiscard]] const RenderStats& getStats() const noexcept { return m_Stats; }

private:
    void initFramebuffer(int width, int height);
    static void setupGlState();

    const se::scene::Camera* m_Camera = nullptr;
    Frustum m_Frustum{};
    ModelRenderer m_ModelRenderer;
    PostProcessRenderer m_PostProcess;
    RenderStats m_Stats;
    bool m_Wireframe = false;

    // Off-screen rendering
    std::optional<Framebuffer> m_SceneFbo;
};

}  // namespace se::render