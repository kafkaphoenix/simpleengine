#include "AssetHandle.h"

#include "../rendering/Material.h"
#include "../rendering/Shader.h"
#include "../rendering/Texture.h"
#include "AssetManager.h"
#include "Model.h"

template <>
std::shared_ptr<Model> AssetHandle<Model>::get() const {
    if (!isValid()) return nullptr;
    return m_AssetManager->getAsset<Model>(m_Id);
}

template <>
std::shared_ptr<Shader> AssetHandle<Shader>::get() const {
    if (!isValid()) return nullptr;
    return m_AssetManager->getAsset<Shader>(m_Id);
}

template <>
std::shared_ptr<Texture> AssetHandle<Texture>::get() const {
    if (!isValid()) return nullptr;
    return m_AssetManager->getAsset<Texture>(m_Id);
}

template <>
std::shared_ptr<Material> AssetHandle<Material>::get() const {
    if (!isValid()) return nullptr;
    return m_AssetManager->getAsset<Material>(m_Id);
}