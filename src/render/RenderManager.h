#pragma once
#include <optional>

#include "Frustum.h"
#include "ModelRenderer.h"
#include "game/Camera.h"
#include "world/LightData.h"
#include "world/Renderable.h"
#include "RenderStats.h"

namespace se::render {

class RenderManager {
   public:
    RenderManager();

    void beginFrame(const se::game::Camera& camera);
    void submit(const se::world::Renderable& renderable);
    void endFrame(const se::world::LightData& lights);

    void toggleWireframe();
    void setBatchSize(size_t maxInstances);
    void reset();

    const RenderStats& getStats() const noexcept;

   private:
    void clear();
    void setupGlState();

    const se::game::Camera* m_Camera = nullptr;
    Frustum m_Frustum{};
    ModelRenderer m_ModelRenderer;
    RenderStats m_Stats;
};

}  // namespace se::render