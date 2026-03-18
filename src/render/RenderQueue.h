#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "Frustum.h"
#include "Mesh.h"
#include "RenderStats.h"
#include "assets/Material.h"
#include "world/Renderable.h"

namespace se::render {

struct InstanceData {
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
};

struct BatchKey {
    Mesh* mesh = nullptr;
    se::assets::Material* material = nullptr;

    struct Hash {
        size_t operator()(const BatchKey& k) const noexcept {
            const size_t h1 = std::hash<Mesh*>{}(k.mesh);
            const size_t h2 = std::hash<se::assets::Material*>{}(k.material);
            return h1 ^ (h2 << 1);
        }
    };

    bool operator==(const BatchKey&) const noexcept = default;
};

struct BatchData {
    std::vector<InstanceData> instances;
    glm::vec3 centerSum{0.0f};
};

using BatchMap = std::unordered_map<BatchKey, BatchData, BatchKey::Hash>;

class RenderQueue {
   public:
    void submit(const se::world::Renderable& renderable, const Frustum& frustum);
    void clear();

    BatchMap& getOpaqueBatches() { return m_OpaqueBatches; }
    BatchMap& getTransparentBatches() { return m_TransparentBatches; }
    const BatchMap& getOpaqueBatches() const { return m_OpaqueBatches; }
    const BatchMap& getTransparentBatches() const { return m_TransparentBatches; }

    const RenderStats& getStats() const noexcept { return m_Stats; }

   private:
    BatchMap m_OpaqueBatches;
    BatchMap m_TransparentBatches;
    RenderStats m_Stats;
};

}  // namespace se::render