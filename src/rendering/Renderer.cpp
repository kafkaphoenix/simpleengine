#include "Renderer.h"

#include <glad/glad.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <stdexcept>

#include "Frustum.h"
#include "Texture.h"

namespace {
struct PointLightUbo {
    glm::vec4 positionRange;
    glm::vec4 colorIntensity;
};

struct FrameUbo {
    glm::mat4 viewProj;
    glm::vec4 sunDir;
    glm::vec4 sunColor;
    glm::vec4 ambient;
    glm::vec4 lightCounts;
    PointLightUbo pointLights[4];
};
}

Renderer::Renderer() {
    setupGlState();
    setupFrameUbo();
    Mesh::setDefaultInstanceCapacityBytes(m_MaxBatchSize * sizeof(InstanceData));
}

void Renderer::setBatchSize(size_t maxInstances) {
    m_MaxBatchSize = maxInstances;
    Mesh::setDefaultInstanceCapacityBytes(m_MaxBatchSize * sizeof(InstanceData));
}

void Renderer::setupGlState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.5f, 1.0f);
}

void Renderer::setupFrameUbo() {
    m_FrameUbo = UniformBuffer(sizeof(FrameUbo), 0);
}

void Renderer::clear() {
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::submit(const Renderable& renderable) {
    if (!renderable.mesh) {
        throw std::runtime_error("Renderable missing mesh");
    }

    auto materialPtr = renderable.material.get();
    if (!materialPtr) {
        throw std::runtime_error("Renderable missing material");
    }

    if (!m_Camera) {
        throw std::runtime_error("Renderer error: No camera set for rendering!");
    }

    glm::mat4 modelMatrix = renderable.transform.getMatrix();
    Frustum frustum = extractFrustum(m_Camera->getViewProjection());
    const AABB& aabb = renderable.mesh->getAABB();
    if (!frustumIntersectsAABB(frustum, aabb, modelMatrix)) {
        return;  // Culled
    }

    BatchKey key{
        renderable.mesh,
        materialPtr.get()};

    InstanceData data;
    data.modelMatrix = modelMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(data.modelMatrix)));
    data.normalMatrix = normalMatrix;

    auto& batch = m_Batches[key];
    batch.instances.push_back(data);

    if (batch.instances.size() >= m_MaxBatchSize) {
        flushBatch(key, batch);
        batch.instances.clear();
    }
}

void Renderer::flushBatch(const BatchKey& key, BatchData& batch) {
    if (batch.instances.empty()) return;

    const RenderState& state = key.material->getState();
    if (state.blend) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    glDepthMask(state.depthWrite ? GL_TRUE : GL_FALSE);
    if (state.cull) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    auto shader = key.material->getShaderHandle().get();
    if (!shader) {
        throw std::runtime_error("Material missing shader");
    }
    shader->bind();

    shader->bindUniformBlock("FrameData", 0);

    auto texture = key.material->getBaseColorHandle().get();
    if (texture) {
        texture->bind(0);
        shader->setInt("u_Texture", 0);
        shader->setBool("u_HasTexture", true);
    } else {
        shader->setBool("u_HasTexture", false);
    }

    const auto& params = key.material->getParams();
    shader->setVec4("u_BaseColorFactor", &params.baseColorFactor[0]);
    shader->setFloat("u_AlphaCutoff", params.alphaCutoff);

    key.mesh->updateInstanceBuffer(
        batch.instances.data(),
        batch.instances.size() * sizeof(InstanceData));

    key.mesh->drawInstanced(batch.instances.size());

    m_Stats.drawCalls++;
    m_Stats.triangles += (key.mesh->getIndexCount() / 3) * batch.instances.size();
}

void Renderer::flush() {
    if (!m_Camera) {
        throw std::runtime_error("Renderer error: No camera set for rendering!");
    }

    m_Stats.reset();

    updateFrameUbo();

    // Flush all remaining batches
    for (auto& [key, batch] : m_Batches) {
        if (!batch.instances.empty()) {
            flushBatch(key, batch);
        }
    }

    m_Batches.clear();

    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Renderer::updateFrameUbo() {
    FrameUbo data{};
    data.viewProj = m_Camera->getViewProjection();

    glm::vec3 sunDir = glm::normalize(m_Lights.sunDir);
    data.sunDir = glm::vec4(sunDir, 0.0f);
    data.sunColor = glm::vec4(m_Lights.sunColor, 0.0f);
    data.ambient = glm::vec4(m_Lights.ambientColor, m_Lights.ambientStrength);

    int pointCount = static_cast<int>(m_Lights.pointLights.size());
    pointCount = std::min(pointCount, 4);
    data.lightCounts = glm::vec4(static_cast<float>(pointCount), 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < pointCount; ++i) {
        const auto& light = m_Lights.pointLights[i];
        data.pointLights[i].positionRange = glm::vec4(light.position, light.range);
        data.pointLights[i].colorIntensity = glm::vec4(light.color, light.intensity);
    }

    m_FrameUbo.updateSubData(0, sizeof(FrameUbo), &data);
}

void Renderer::reset() {
    m_Batches.clear();
    m_Stats.reset();
}

void Renderer::toggleWireframe() {
    static bool wireframe = false;
    wireframe = !wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}
