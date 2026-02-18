#pragma once
#include <cstdint>
#include <memory>

class AssetManager;
class Asset;
class Model;
class Shader;
class Texture;
class Material;

template <typename T>
class AssetHandle {
   public:
    AssetHandle() : m_AssetManager(nullptr), m_Id(0) {}
    AssetHandle(AssetManager* manager, uint64_t id)
        : m_AssetManager(manager), m_Id(id) {}

    std::shared_ptr<T> get() const;

    bool isValid() const { return m_AssetManager != nullptr && m_Id != 0; }

    uint64_t getId() const { return m_Id; }

    bool operator==(const AssetHandle<T>& other) const {
        return m_AssetManager == other.m_AssetManager && m_Id == other.m_Id;
    }
    bool operator!=(const AssetHandle<T>& other) const {
        return !(*this == other);
    }

   private:
    AssetManager* m_AssetManager;
    uint64_t m_Id;
};

using ModelHandle = AssetHandle<Model>;
using ShaderHandle = AssetHandle<Shader>;
using TextureHandle = AssetHandle<Texture>;
using MaterialHandle = AssetHandle<Material>;

namespace std {
template <typename T>
struct hash<AssetHandle<T>> {
    std::size_t operator()(const AssetHandle<T>& handle) const {
        return std::hash<uint64_t>{}(handle.getId());
    }
};
}