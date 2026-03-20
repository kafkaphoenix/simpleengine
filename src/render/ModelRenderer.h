#pragma once
#include <glm/glm.hpp>
#include <optional>
#include <vector>

#include "Frustum.h"
#include "Mesh.h"
#include "RenderQueue.h"
#include "RenderStats.h"
#include "UniformBuffer.h"
#include "world/Camera.h"
#include "world/LightData.h"
#include "world/Renderable.h"

namespace se::render {

class ModelRenderer {
   public:
    ModelRenderer();

    void submit(const se::world::Renderable& renderable, const Frustum& frustum);
    void flush(const se::world::LightData& lights, const se::world::Camera& camera);
    void toggleWireframe();
    void setBatchSize(size_t maxInstances);
    void reset();

    const RenderStats& getStats() const noexcept { return m_Stats; }

   private:
    struct TransparentDraw {
        float distance = 0.0f;
        BatchKey key;
        BatchData* batch = nullptr;
    };

    void setupFrameUbo();
    void flushBatch(const BatchKey& key, BatchData& batch);
    void updateFrameUbo(const se::world::LightData& lights,
                        const se::world::Camera& camera);
    void resetGlState();
    void applyWireframeState();

    std::vector<TransparentDraw> getSortedTransparentDraws(
        const se::world::Camera& camera);

    RenderQueue m_Queue;
    size_t m_MaxBatchSize = 1000;
    std::optional<UniformBuffer> m_FrameUbo;
    bool m_Wireframe = false;
    RenderStats m_Stats;
};

}  // namespace se::render