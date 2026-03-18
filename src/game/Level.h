#pragma once
#include "Player.h"
#include "assets/AssetManager.h"
#include "world/World.h"
#include "world/WorldLoader.h"

namespace se::core {
class Input;
class Config;
}
namespace se::render {
class RenderManager;
}

namespace se::game {

class Level {
   public:
    Level(float aspectRatio, se::assets::AssetManager& assetManager);
    ~Level() = default;

    se::world::World& getWorld() { return m_World; }
    const se::world::World& getWorld() const { return m_World; }
    Player& getPlayer() { return m_Player; }
    const Player& getPlayer() const { return m_Player; }

    void initialize();
    void update(float deltaTime, const se::core::Input& input);
    void render(se::render::RenderManager& renderManager);
    void applyConfig(const se::core::Config& config);

   private:
    se::world::World m_World;
    Player m_Player;
    se::world::WorldLoader m_Loader;
};

}  // namespace se::game