#pragma once
#include <optional>

#include "Frustum.h"
#include "ModelRenderer.h"
#include "RenderStats.h"
#include "world/Camera.h"
#include "world/LightData.h"
#include "world/Renderable.h"

namespace se::render {

class RenderManager {
   public:
    RenderManager();

    void beginFrame(const se::world::Camera& camera);
    void submit(const se::world::Renderable& renderable);
    void endFrame(const se::world::LightData& lights);

    void toggleWireframe();
    void setBatchSize(size_t maxInstances);
    void reset();

    const RenderStats& getStats() const noexcept;

   private:
    void clear();
    void setupGlState();

    const se::world::Camera* m_Camera = nullptr;
    Frustum m_Frustum{};
    ModelRenderer m_ModelRenderer;
    RenderStats m_Stats;
};

}  // namespace se::render