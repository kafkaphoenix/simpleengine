#include "Level.h"

#include "Config.h"
#include "Input.h"
#include "assets/AssetManager.h"
#include "render/RenderManager.h"
#include "scene/AnimatedActor.h"
#include "scene/SceneBuilder.h"

namespace se::core {

Level::Level(const Config& config, se::render::RenderManager& renderManager, se::assets::AssetManager& assetManager)
    : m_Player(config) {
    se::scene::SceneBuilder::build(m_Scene, assetManager, renderManager);
    m_Player.setBodyActor(m_Scene.findActor("player_body"));
}

void Level::update(float deltaTime, const Input& input) {
    m_Player.update(deltaTime, input);
    m_Scene.update(deltaTime);
}

void Level::render(se::render::RenderManager& renderManager) {
    renderManager.beginFrame(m_Player.getCamera());

    for (const auto& r : m_Scene.getRenderables()) { renderManager.submit(r); }

    for (const auto& actorPtr : m_Scene.getAnimatedActors()) {
        for (const auto& ar : actorPtr->collectRenderables()) {
            renderManager.submitAnimated(ar.renderable, ar.boneMatrices);
        }
    }

    renderManager.endFrame(m_Scene.prepareLightData());
}

}  // namespace se::core