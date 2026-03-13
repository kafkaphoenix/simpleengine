#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Frustum.h"
#include "Mesh.h"
#include "UniformBuffer.h"
#include "assets/Material.h"
#include "assets/Shader.h"
#include "scene/Camera.h"
#include "scene/Renderable.h"

namespace se::scene {
class Scene;
}

namespace se::render {

struct InstanceData {
    glm::mat4 modelMatrix;   // locations 3-6  (64 bytes)
    glm::mat3 normalMatrix;  // locations 7-9  (36 bytes)
};

struct BatchKey {
    Mesh* mesh;
    se::assets::Material* material;

    struct Hash {
        size_t operator()(const BatchKey& key) const {
            size_t h1 = std::hash<Mesh*>{}(key.mesh);
            size_t h2 = std::hash<se::assets::Material*>{}(key.material);
            return h1 ^ (h2 << 1);
        }
    };

    bool operator==(const BatchKey& other) const {
        return mesh == other.mesh && material == other.material;
    }
};

struct BatchData {
    std::vector<InstanceData> instances;
    glm::vec3 centerSum{0.0f};
};

class Renderer {
   public:
    struct PointLightData {
        glm::vec3 position{0.0f};
        glm::vec3 color{1.0f};
        float intensity = 1.0f;
        float range = 25.0f;
    };

    struct LightSet {
        glm::vec3 sunDir{0.0f, -1.0f, 0.0f};
        glm::vec3 sunColor{1.0f, 1.0f, 1.0f};
        glm::vec3 ambientColor{1.0f, 1.0f, 1.0f};
        float ambientStrength = 0.2f;
        std::vector<PointLightData> pointLights;
    };

    Renderer();

    void render(const se::scene::Scene& scene);
    void setCamera(const se::scene::Camera& camera) { m_Camera = &camera; }
    void clear();
    void submit(const se::scene::Renderable& renderable);
    void flush();
    void toggleWireframe();
    void setLights(const LightSet& lights) { m_Lights = lights; }
    void setBatchSize(size_t maxInstances);
    void reset();

    struct Stats {
        unsigned int drawCalls = 0;
        unsigned int triangles = 0;
        void reset() { drawCalls = triangles = 0; }
    } m_Stats;

    const Stats& getStats() const { return m_Stats; }

   private:
    struct TransparentDraw {
        float distance;
        BatchKey key;
        BatchData* batch;
    };

    void setupGlState();
    void setupFrameUbo();
    void flushBatch(const BatchKey& key, BatchData& batch);
    void updateFrameUbo();
    void resetGlState();
    void applyWireframeState();
    void clearBatches();
    std::vector<TransparentDraw> getSortedTransparentDraws();

    const se::scene::Camera* m_Camera = nullptr;
    std::unordered_map<BatchKey, BatchData, BatchKey::Hash> m_OpaqueBatches;
    std::unordered_map<BatchKey, BatchData, BatchKey::Hash> m_TransparentBatches;
    size_t m_MaxBatchSize = 1000;
    LightSet m_Lights;
    // std::optional avoids constructing a real 0-size GL buffer object at member init
    std::optional<UniformBuffer> m_FrameUbo;
    Frustum m_Frustum{};
    bool m_Wireframe = false;
};

}  // namespace se::render