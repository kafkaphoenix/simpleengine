#include "Scene.h"

#include <stdexcept>

#include "core/Input.h"

Scene::Scene(float aspectRatio, AssetManager& assetManager) : m_Player(aspectRatio), m_AssetManager(assetManager) {
}

void Scene::initialize() {
    createSponzaModel();
}

void Scene::createSponzaModel() {
    Transform t;
    t.position = {0.0f, 0.0f, 0.0f};
    t.scale = {0.1f, 0.1f, 0.1f};
    std::string shaderPath = "assets/shaders/basic";
    std::string modelPath = "assets/models/sponza/sponza.gltf";
    auto shader = m_AssetManager.getOrLoadShader(shaderPath);
    auto model = m_AssetManager.getOrLoadModel(modelPath, shaderPath);

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
        renderable.transform = t;
        addRenderable(renderable);
    }
}

void Scene::update(float deltaTime, const Input& input) {
    m_Player.update(deltaTime, input);
}
