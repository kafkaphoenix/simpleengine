#pragma once
#include "assets/AssetManager.h"
#include "world/Player.h"
#include "world/World.h"
#include "world/WorldLoader.h"

namespace se::core {
class Input;
class Config;
}
namespace se::render {
class RenderManager;
}

namespace se::scene {

class Level {
   public:
    Level(float aspectRatio, se::assets::AssetManager& assetManager);
    ~Level() = default;

    se::world::World& getWorld() { return m_World; }
    const se::world::World& getWorld() const { return m_World; }
    se::world::Player& getPlayer() { return m_Player; }
    const se::world::Player& getPlayer() const { return m_Player; }

    void initialize();
    void update(float deltaTime, const se::core::Input& input);
    void render(se::render::RenderManager& renderManager);
    void applyConfig(const se::core::Config& config);

   private:
    se::world::World m_World;
    se::world::Player m_Player;
    se::world::WorldLoader m_Loader;
};

}  // namespace se::scene