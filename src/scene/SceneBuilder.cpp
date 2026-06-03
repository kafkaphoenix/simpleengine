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
    std::println("Static models loaded in {} ms", timer.millis());
    submitModel(model, Transform{.position = {0.0f, 0.0f, 0.0f}, .scale = {0.1f, 0.1f, 0.1f}}, scene);

    timer.reset();
    auto animShader = assetManager.getOrLoadShader("assets/shaders/animated_model", "assets/shaders/model");
    auto handle = assetManager.getOrLoadModel("assets/models/fox.glb", animShader);
    std::println("Animated models loaded in {} ms", timer.millis());
    submitModel(handle, Transform{.scale = {0.1f, 0.1f, 0.1f}}, scene, "player_body",
                AnimationController::LocomotionClips{.idle = "Survey", .walk = "Walk", .run = "Run"});
}

void SceneBuilder::submitModel(const se::assets::ModelHandle& handle, const Transform& transform, Scene& scene,
                               std::optional<std::string> animatedTag,
                               std::optional<AnimationController::LocomotionClips> locomotionClips) {
    auto model = handle.get();
    if (!model) {
        throw std::runtime_error("Model handle is invalid");
    }

    AnimatedInstance* animatedInstance = nullptr;
    if (animatedTag.has_value()) {
        animatedInstance = &scene.addAnimatedInstance(
            AnimatedInstance(handle, transform, std::move(*animatedTag),
                             locomotionClips.value_or(AnimationController::LocomotionClips{})));
    }

    for (const auto& sub : model->getSubMeshes()) {
        if (!sub.mesh) {
            throw std::runtime_error("SubMesh is missing mesh data");
        }

        if (animatedInstance) {
            scene.addRenderable(Renderable::makeAnimated(sub.mesh.get(), sub.material, animatedInstance->transform,
                                                         animatedInstance->animator));
        } else {
            scene.addRenderable(Renderable::makeStatic(sub.mesh.get(), sub.material, transform));
        }
    }
}
}  // namespace se::scene