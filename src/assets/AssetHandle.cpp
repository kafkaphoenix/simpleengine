#include "AssetHandle.h"

#include "AssetManager.h"
#include "Material.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"

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