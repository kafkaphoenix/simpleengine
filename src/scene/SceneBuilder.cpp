#include "SceneBuilder.h"

#include <glm/glm.hpp>
#include <print>
#include <stdexcept>

#include "Light.h"
#include "Renderable.h"
#include "Scene.h"
#include "Sun.h"
#include "Transform.h"
#include "assets/AssetManager.h"
#include "assets/Cubemap.h"
#include "core/Timer.h"
#include "render/RenderManager.h"

namespace se::scene {

void SceneBuilder::build(Scene& scene, se::assets::AssetManager& assetManager,
                         se::render::RenderManager& renderManager) {
    createSky(scene, assetManager, renderManager);
    loadModels(scene, assetManager);
    loadAnimatedModels(scene, assetManager);
}

void SceneBuilder::createSky(Scene& scene, se::assets::AssetManager& assetManager,
                             se::render::RenderManager& renderManager) {
    scene.addDirectionalLight(DirectionalLight{
        .direction = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)),
        .color = glm::vec3(1.0f, 0.95f, 0.9f),
        .intensity = 1.0f,
    });

    auto& sky = scene.getSky();
    sky.setAmbientColor(glm::vec3(1.0f, 1.0f, 1.0f));
    sky.setAmbientStrength(0.7f);

    // +X, -X, +Y, -Y, +Z, -Z)
    const std::string dir = "assets/textures/skybox/";
    sky.setCubemapFaces({
        dir + "right.png",
        dir + "left.png",
        dir + "top.png",
        dir + "bottom.png",
        dir + "front.png",
        dir + "back.png",
    });

    if (const auto& faces = sky.getCubemapFaces()) {
        auto cubemap = std::make_shared<se::assets::Cubemap>(*faces);
        renderManager.setSkybox(std::move(cubemap));
    }
}

void SceneBuilder::loadModels(Scene& scene, se::assets::AssetManager& assetManager) {
    se::core::Timer timer;
    auto shader = assetManager.getOrLoadShader("assets/shaders/model");
    const auto& model = assetManager.getOrLoadModel("assets/models/sponza.glb", shader);
    std::println("Sponza loaded in {} ms", timer.millis());
    submitModel(model, Transform{.position = {0.0f, 0.0f, 0.0f}, .scale = {0.1f, 0.1f, 0.1f}}, scene);
}

void SceneBuilder::submitModel(const se::assets::ModelHandle& model, const Transform& transform, Scene& scene) {
    auto modelPtr = model.get();
    if (!modelPtr) {
        throw std::runtime_error("Model handle is invalid");
    }

    for (const auto& sub : modelPtr->getSubMeshes()) {
        if (!sub.mesh) {
            throw std::runtime_error("SubMesh is missing mesh data");
        }

        scene.addRenderable(Renderable{
            .mesh = sub.mesh.get(),
            .material = sub.material,
            .transform = transform,
        });
    }
}

void SceneBuilder::loadAnimatedModels(Scene& scene, se::assets::AssetManager& assetManager) {
    auto animShader = assetManager.getOrLoadShader("assets/shaders/animated_model", "assets/shaders/model");
    se::core::Timer timer;
    auto model = assetManager.getOrLoadModel("assets/models/fox.glb", animShader);
    std::println("fox.glb loaded in {} ms", timer.millis());
    submitAnimatedModel(model, Transform{.scale = {0.1f, 0.1f, 0.1f}}, "player_body", scene);
}

void SceneBuilder::submitAnimatedModel(const se::assets::ModelHandle& model, const Transform& transform,
                                       std::string tag, Scene& scene) {
    scene.addAnimatedActor(AnimatedActor(model, transform, std::move(tag)));
}
}  // namespace se::scene