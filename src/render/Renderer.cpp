#include "Renderer.h"

#include <glad/glad.h>

#include <algorithm>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp>
#include <stdexcept>

#include "Frustum.h"
#include "assets/Texture.h"
#include "scene/Scene.h"

namespace se::render {

namespace {

struct PointLightGpuData {
    glm::vec4 positionRange;
    glm::vec4 colorIntensity;
};

struct FrameUbo {
    glm::mat4 viewProj;
    glm::vec4 sunDir;
    glm::vec4 sunColor;
    glm::vec4 ambient;
    glm::vec4 lightCounts;
    PointLightGpuData pointLights[4];
};

}  // namespace

Renderer::Renderer() {
    setupGlState();
    setupFrameUbo();
    Mesh::setDefaultInstanceCapacityBytes(m_MaxBatchSize * sizeof(InstanceData));
}

void Renderer::render(const se::scene::Scene& scene) {
    m_Stats.reset();

    if (!m_Camera)
        throw std::runtime_error("Renderer error: No camera set for rendering!");

    m_Frustum = extractFrustum(m_Camera->getViewProjection());

    setLights(scene.buildLightSet());
    clear();
    for (const auto& renderable : scene.getRenderables())
        submit(renderable);
    flush();
}

void Renderer::setBatchSize(size_t maxInstances) {
    assert(m_OpaqueBatches.empty() && m_TransparentBatches.empty() &&
           "setBatchSize called mid-frame with live batches");
    m_MaxBatchSize = maxInstances;
    Mesh::setDefaultInstanceCapacityBytes(m_MaxBatchSize * sizeof(InstanceData));
}

void Renderer::setupGlState() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::resetGlState() {
    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    applyWireframeState();
}

void Renderer::applyWireframeState() {
    glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
    if (m_Wireframe) {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.5f, 1.0f);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_LINE_SMOOTH);
    }
}

void Renderer::setupFrameUbo() {
    m_FrameUbo.emplace(sizeof(FrameUbo), 0);
}

void Renderer::clear() {
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::submit(const se::scene::Renderable& renderable) {
    if (!renderable.mesh)
        throw std::runtime_error("Renderable missing mesh");

    auto materialPtr = renderable.material.get();
    if (!materialPtr)
        throw std::runtime_error("Renderable missing material");

    glm::mat4 modelMatrix = renderable.transform.getMatrix();

    const AABB& aabb = renderable.mesh->getAABB();
    if (!frustumIntersectsAABB(m_Frustum, aabb, modelMatrix)) return;

    const auto& state = materialPtr->getState();
    BatchKey key{renderable.mesh, materialPtr.get()};

    InstanceData data;
    data.modelMatrix = modelMatrix;
    data.normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    auto& map = state.blend ? m_TransparentBatches : m_OpaqueBatches;
    auto& batch = map[key];
    batch.instances.push_back(data);
    batch.centerSum += glm::vec3(modelMatrix[3]);  // accumulate world position
}

void Renderer::flushBatch(const BatchKey& key, BatchData& batch) {
    if (batch.instances.empty()) return;

    const auto& state = key.material->getState();
    if (state.cull)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    auto shader = key.material->getShaderHandle().get();
    if (!shader) throw std::runtime_error("Material missing shader");
    shader->bind();

    auto tex = key.material->getBaseColorHandle().get();
    tex->bind(0);

    const auto& params = key.material->getParams();
    shader->setVec4("u_BaseColorFactor", &params.baseColorFactor[0]);
    shader->setFloat("u_AlphaCutoff", params.alphaCutoff);

    key.mesh->updateInstanceBuffer(batch.instances.data(),
                                   batch.instances.size() * sizeof(InstanceData));
    key.mesh->drawInstanced(batch.instances.size());

    m_Stats.drawCalls++;
    m_Stats.triangles += (key.mesh->getIndexCount() / 3) * batch.instances.size();
}

std::vector<Renderer::TransparentDraw> Renderer::getSortedTransparentDraws() {
    std::vector<TransparentDraw> draws;
    draws.reserve(m_TransparentBatches.size());
    glm::vec3 camPos = m_Camera->getPosition();

    for (auto& [key, batch] : m_TransparentBatches) {
        if (batch.instances.empty()) continue;
        glm::vec3 center = batch.centerSum / static_cast<float>(batch.instances.size());
        float dist = glm::length2(camPos - center);
        draws.push_back({dist, key, &batch});
    }

    std::sort(draws.begin(), draws.end(),
              [](const TransparentDraw& a, const TransparentDraw& b) {
                  return a.distance > b.distance;
              });
    return draws;
}

void Renderer::clearBatches() {
    for (auto& [key, batch] : m_OpaqueBatches) {
        batch.instances.clear();
        batch.centerSum = {};
    }
    for (auto& [key, batch] : m_TransparentBatches) {
        batch.instances.clear();
        batch.centerSum = {};
    }
}

// We sort transparent batches back-to-front based on the average world position of their instances.
// It's not possible to sort individual instances without breaking batching
void Renderer::flush() {
    if (!m_Camera) throw std::runtime_error("Renderer has no camera!");

    updateFrameUbo();

    // Opaque pass — depth writes on, blending off
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    for (auto& [key, batch] : m_OpaqueBatches)
        flushBatch(key, batch);

    // Transparent pass — sorted back-to-front, depth writes off
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    for (auto& draw : getSortedTransparentDraws())
        flushBatch(draw.key, *draw.batch);

    clearBatches();
    resetGlState();
}

void Renderer::updateFrameUbo() {
    FrameUbo data{};
    data.viewProj = m_Camera->getViewProjection();

    glm::vec3 sunDir = glm::normalize(m_Lights.sunDir);
    data.sunDir = glm::vec4(sunDir, 0.0f);
    data.sunColor = glm::vec4(m_Lights.sunColor, 0.0f);
    data.ambient = glm::vec4(m_Lights.ambientColor, m_Lights.ambientStrength);

    int pointCount = std::min(static_cast<int>(m_Lights.pointLights.size()), 4);
    data.lightCounts = glm::vec4(static_cast<float>(pointCount), 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < pointCount; ++i) {
        const auto& light = m_Lights.pointLights[i];
        data.pointLights[i].positionRange = glm::vec4(light.position, light.range);
        data.pointLights[i].colorIntensity = glm::vec4(light.color, light.intensity);
    }

    m_FrameUbo->updateSubData(0, sizeof(FrameUbo), &data);
}

void Renderer::reset() {
    clearBatches();
    m_Stats.reset();
}

void Renderer::toggleWireframe() {
    m_Wireframe = !m_Wireframe;
    applyWireframeState();
}

}  // namespace se::render