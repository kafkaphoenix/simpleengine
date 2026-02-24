#include "AssetManager.h"

#include <stdexcept>

#include "../rendering/Material.h"
#include "Asset.h"

template <>
AssetHandle<Model> AssetManager::loadAsset(const std::string& path, const std::string& shaderPath) {
    uint64_t id = generateId();
    try {
        auto model = std::make_shared<Model>(path, shaderPath, *this);
        m_Assets[id] = model;
        return AssetHandle<Model>(this, id);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load model '" + path + "': " + std::string(e.what()));
    }
}

template <>
AssetHandle<Shader> AssetManager::loadAsset<Shader>(const std::string& shaderPath) {
    uint64_t id = generateId();

    try {
        auto shader = std::make_shared<Shader>(shaderPath);
        m_Assets[id] = shader;
        return AssetHandle<Shader>(this, id);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load shader '" + shaderPath + "': " + std::string(e.what()));
    }
}

template <>
AssetHandle<Texture> AssetManager::loadAsset<Texture>(const std::string& path) {
    auto pathIt = m_TexturePathToId.find(path);
    if (pathIt != m_TexturePathToId.end()) {
        uint64_t existingId = pathIt->second;
        return AssetHandle<Texture>(this, existingId);
    }

    uint64_t id = generateId();

    try {
        auto texture = std::make_shared<Texture>(path);
        m_Assets[id] = texture;

        m_TexturePathToId[path] = id;
        return AssetHandle<Texture>(this, id);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load texture '" + path + "': " + std::string(e.what()));
    }
}

ShaderHandle AssetManager::loadShader(const std::string& shaderPath) {
    return loadAsset<Shader>(shaderPath);
}

ModelHandle AssetManager::loadModel(const std::string& gltfPath, const std::string& shaderPath) {
    return loadAsset<Model>(gltfPath, shaderPath);
}

TextureHandle AssetManager::loadTexture(const std::string& path) {
    return loadAsset<Texture>(path);
}

MaterialHandle AssetManager::createMaterial(const std::string& name,
                                            ShaderHandle shader,
                                            const MaterialTextures& textures,
                                            const MaterialParams& params,
                                            const RenderState& state) {
    uint64_t id = generateId();
    auto material = std::make_shared<Material>(name, shader, textures, params, state);
    m_Assets[id] = material;
    return MaterialHandle(this, id);
}

TextureHandle AssetManager::getTextureHandle(const std::string& path) const {
    auto pathIt = m_TexturePathToId.find(path);
    if (pathIt != m_TexturePathToId.end()) {
        return TextureHandle(const_cast<AssetManager*>(this), pathIt->second);
    }
    return TextureHandle();  // Invalid handle
}

template <>
std::shared_ptr<Model> AssetManager::getAsset<Model>(uint64_t id) const {
    auto it = m_Assets.find(id);
    if (it != m_Assets.end()) {
        return std::dynamic_pointer_cast<Model>(it->second);
    }
    return nullptr;
}

template <>
std::shared_ptr<Shader> AssetManager::getAsset<Shader>(uint64_t id) const {
    auto it = m_Assets.find(id);
    if (it != m_Assets.end()) {
        return std::dynamic_pointer_cast<Shader>(it->second);
    }
    return nullptr;
}

template <>
std::shared_ptr<Texture> AssetManager::getAsset<Texture>(uint64_t id) const {
    auto it = m_Assets.find(id);
    if (it != m_Assets.end()) {
        return std::dynamic_pointer_cast<Texture>(it->second);
    }
    return nullptr;
}

template <>
std::shared_ptr<Material> AssetManager::getAsset<Material>(uint64_t id) const {
    auto it = m_Assets.find(id);
    if (it != m_Assets.end()) {
        return std::dynamic_pointer_cast<Material>(it->second);
    }
    return nullptr;
}

void AssetManager::clear() {
    m_Assets.clear();

    m_TexturePathToId.clear();
}