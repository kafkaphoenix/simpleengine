#include "Level.h"

#include "core/Config.h"
#include "core/Input.h"
#include "render/RenderManager.h"

namespace se::scene {

Level::Level(float aspectRatio, se::assets::AssetManager& assetManager)
    : m_Player(aspectRatio), m_Loader(assetManager) {}

void Level::initialize() {
    m_Loader.load(m_World);
}

void Level::update(float deltaTime, const se::core::Input& input) {
    m_Player.update(deltaTime, input);
}

void Level::render(se::render::RenderManager& renderManager) {
    renderManager.beginFrame(m_Player.getCamera());
    for (const auto& r : m_World.getRenderables())
        renderManager.submit(r);
    renderManager.endFrame(m_World.getLightData());
}

void Level::applyConfig(const se::core::Config& config) {
    m_Player.applyConfig(config);
}

}  // namespace se::scene