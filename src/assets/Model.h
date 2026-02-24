#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "Asset.h"

class AssetManager;

struct SubMesh {
    std::unique_ptr<Mesh> mesh;
    MaterialHandle material;
};

class Model : public Asset {
   public:
    Model(const std::string& gltfPath, const std::string& shaderPath, AssetManager& assetManager);
    const std::vector<SubMesh>& getSubMeshes() const { return m_SubMeshes; }
    const std::string& getPath() const override { return m_Path; }

   private:
    std::vector<SubMesh> m_SubMeshes;

    std::string getDirectory(const std::string& filepath) const;
};
