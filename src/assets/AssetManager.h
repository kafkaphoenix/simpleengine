#pragma once
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

#include "../rendering/Material.h"
#include "../rendering/Shader.h"
#include "../rendering/Texture.h"
#include "Asset.h"
#include "AssetHandle.h"
#include "Model.h"

class AssetManager {
   public:
    AssetManager() : m_NextId(1) {}  // Start IDs from 1 (0 means invalid)
    ~AssetManager() = default;

    template <typename T, typename... Args>
    AssetHandle<T> loadAsset(const std::string& path, Args&&... args);

    ShaderHandle loadShader(const std::string& shaderPath);
    ModelHandle loadModel(const std::string& gltfPath, const std::string& shaderPath);
    TextureHandle loadTexture(const std::string& path);
    MaterialHandle createMaterial(const std::string& name,
                                  ShaderHandle shader,
                                  const MaterialTextures& textures,
                                  const MaterialParams& params,
                                  const RenderState& state);

    TextureHandle getTextureHandle(const std::string& path) const;

    void clear();

   private:
    template <typename T>
    std::shared_ptr<T> getAsset(uint64_t id) const;

   private:
    std::unordered_map<uint64_t, std::shared_ptr<Asset>> m_Assets;
    std::atomic<uint64_t> m_NextId;

    std::unordered_map<std::string, uint64_t> m_TexturePathToId;

    uint64_t generateId() { return m_NextId.fetch_add(1); }

    template <typename T>
    friend class AssetHandle;
};