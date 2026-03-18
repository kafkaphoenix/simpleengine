#pragma once

namespace se::assets {
class AssetManager;
}

namespace se::world {

class World;

class WorldLoader {
   public:
    explicit WorldLoader(se::assets::AssetManager& assetManager);

    void load(World& world);

   private:
    void loadSky(World& world);
    void loadModels(World& world);

    se::assets::AssetManager& m_AssetManager;
};

}  // namespace se::world