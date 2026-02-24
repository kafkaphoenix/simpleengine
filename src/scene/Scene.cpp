#include "Scene.h"

#include <stdexcept>

#include "../core/Input.h"

Scene::Scene(float aspectRatio, AssetManager& assetManager) : m_Player(aspectRatio), m_AssetManager(assetManager) {
}

void Scene::initialize() {
    createSponzaModel();
}

void Scene::createSponzaModel() {
    Transform t;
    t.position = {0.0f, 0.0f, 0.0f};
    t.scale = {0.1f, 0.1f, 0.1f};
    ShaderHandle shader = m_AssetManager.loadShader("assets/shaders/basic");
    createModelInstance("assets/models/sponza/sponza.gltf", shader.get()->getPath(), t);
}

void Scene::update(float deltaTime, const Input& input) {
    m_Player.update(deltaTime, input);
}

void Scene::createModelInstance(const std::string& modelPath, const std::string& shaderPath, const Transform& transform) {
    auto model = m_AssetManager.loadModel(modelPath, shaderPath);
    createRenderablesFromModel(model, transform);
}

void Scene::createRenderablesFromModel(ModelHandle model, const Transform& transform) {
    auto modelPtr = model.get();
    if (!modelPtr) {
        throw std::runtime_error("Model handle is invalid");
    }

    for (const auto& sub : modelPtr->getSubMeshes()) {
        if (!sub.mesh) {
            throw std::runtime_error("SubMesh is missing mesh data");
        }
        Renderable renderable;
        renderable.mesh = sub.mesh.get();
        renderable.material = sub.material;
        renderable.transform = transform;
        addRenderable(renderable);
    }
}