#pragma once
#include <memory>
#include <vector>

#include "assets/AssetManager.h"
#include "assets/Model.h"
#include "Player.h"
#include "Renderable.h"
#include "Sky.h"

class Input;
class Scene {
   public:
    Scene(float aspectRatio, AssetManager& assetManager);
    ~Scene() = default;

    void addRenderable(const Renderable& renderable) { m_Renderables.push_back(renderable); }
    const std::vector<Renderable>& getRenderables() const { return m_Renderables; }

    Player& getPlayer() { return m_Player; }
    const Player& getPlayer() const { return m_Player; }

    Sky& getSky() { return m_Sky; }
    const Sky& getSky() const { return m_Sky; }

    const std::vector<Light>& getPointLights() const { return m_PointLights; }
    std::vector<Light>& getPointLights() { return m_PointLights; }

    AssetManager& getAssetManager() { return m_AssetManager; }
    const AssetManager& getAssetManager() const { return m_AssetManager; }

    void update(float deltaTime, const Input& input);
    void initialize();
    void createSponzaModel();

   private:
    void createModelInstance(const std::string& modelPath, const std::string& shaderPath, const Transform& transform);
    void createRenderablesFromModel(ModelHandle model, const Transform& transform);

    std::vector<Renderable> m_Renderables;
    Player m_Player;
    Sky m_Sky;
    std::vector<Light> m_PointLights;
    AssetManager& m_AssetManager;
};
