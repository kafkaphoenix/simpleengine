#pragma once

#include <optional>
#include <string>

#include "assets/AssetHandle.h"

namespace se::assets {
class AssetManager;
class Model;
}

namespace se::render {
class RenderManager;
}

namespace se::scene {

class Scene;
struct Transform;

class SceneBuilder {
public:
    SceneBuilder() = delete;

    static void build(Scene& scene, se::assets::AssetManager& assetManager, se::render::RenderManager& renderManager);

private:
    static void createSky(Scene& scene, se::assets::AssetManager& assetManager,
                          se::render::RenderManager& renderManager);
    static void loadModels(Scene& scene, se::assets::AssetManager& assetManager);
    static void submitModel(const se::assets::ModelHandle& model, const Transform& transform, Scene& scene,
                            std::optional<std::string> animatedTag = std::nullopt);
};

}  // namespace se::scene