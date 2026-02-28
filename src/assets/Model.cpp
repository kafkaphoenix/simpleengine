#include "Model.h"

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <cstdint>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <stdexcept>

#include "AssetManager.h"

namespace {

tinygltf::Model loadGltfModel(const std::string& gltfPath) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = (gltfPath.find(".glb") != std::string::npos)
                   ? loader.LoadBinaryFromFile(&gltfModel, &err, &warn, gltfPath)
                   : loader.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfPath);

    if (!ret) throw std::runtime_error("Failed to load GLTF: " + err);
    if (!warn.empty()) std::cout << "GLTF Warning: " << warn << std::endl;

    return gltfModel;
}

std::vector<TextureHandle> loadGltfTextures(const tinygltf::Model& gltfModel,
                                            const std::string& gltfDir,
                                            AssetManager& assetManager) {
    std::vector<TextureHandle> gltfTextures;
    gltfTextures.reserve(gltfModel.textures.size());

    for (const auto& texture : gltfModel.textures) {
        if (texture.source < 0 || texture.source >= static_cast<int>(gltfModel.images.size()))
            throw std::runtime_error("Invalid texture source: " + std::to_string(texture.source));

        const auto& image = gltfModel.images[texture.source];
        if (image.uri.empty())
            throw std::runtime_error("Embedded textures not supported: " + std::to_string(texture.source));

        std::string path = gltfDir + "/" + image.uri;
        auto handle = assetManager.getOrLoadTexture(path);
        if (!handle.isValid())
            throw std::runtime_error("Failed to load texture: " + path);

        gltfTextures.push_back(handle);
    }

    return gltfTextures;
}

MaterialHandle createDefaultMaterial(const std::string& name,
                                     AssetManager& assetManager,
                                     const ShaderHandle& shader) {
    MaterialTextures defaultTextures;
    MaterialParams defaultParams;
    return assetManager.getOrLoadMaterial(name, shader, defaultTextures, defaultParams, RenderState{});
}

std::vector<MaterialHandle> buildMaterials(const tinygltf::Model& gltfModel,
                                           AssetManager& assetManager,
                                           const ShaderHandle& shader,
                                           const std::vector<TextureHandle>& textures) {
    std::vector<MaterialHandle> materials;
    materials.reserve(gltfModel.materials.size());

    for (size_t i = 0; i < gltfModel.materials.size(); ++i) {
        const auto& mat = gltfModel.materials[i];
        MaterialTextures matTextures;
        MaterialParams params;

        auto assignTexture = [&](int texIndex, TextureHandle& dst) {
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size()))
                dst = textures[texIndex];
        };

        assignTexture(mat.pbrMetallicRoughness.baseColorTexture.index, matTextures.baseColor);
        assignTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, matTextures.metallicRoughness);
        assignTexture(mat.normalTexture.index, matTextures.normal);
        assignTexture(mat.emissiveTexture.index, matTextures.emissive);
        assignTexture(mat.occlusionTexture.index, matTextures.occlusion);

        if (mat.pbrMetallicRoughness.baseColorFactor.size() == 4) {
            params.baseColorFactor = glm::vec4(
                static_cast<float>(mat.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(mat.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(mat.pbrMetallicRoughness.baseColorFactor[2]),
                static_cast<float>(mat.pbrMetallicRoughness.baseColorFactor[3]));
        }
        params.metallicFactor = static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);
        params.roughnessFactor = static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);

        if (mat.emissiveFactor.size() == 3) {
            params.emissiveFactor = glm::vec3(
                static_cast<float>(mat.emissiveFactor[0]),
                static_cast<float>(mat.emissiveFactor[1]),
                static_cast<float>(mat.emissiveFactor[2]));
        }

        params.alphaCutoff = (mat.alphaMode == "MASK") ? static_cast<float>(mat.alphaCutoff) : 0.0f;

        RenderState state;
        state.blend = (mat.alphaMode == "BLEND");
        state.depthWrite = !state.blend;
        state.cull = !mat.doubleSided;

        std::string matName = mat.name.empty() ? "material_" + std::to_string(i) : mat.name;
        materials.push_back(assetManager.getOrLoadMaterial(matName, shader, matTextures, params, state));
    }

    return materials;
}

std::vector<unsigned int> readIndices(const tinygltf::Model& gltfModel,
                                      const tinygltf::Primitive& primitive,
                                      size_t vertexCount) {
    std::vector<unsigned int> indices;

    if (primitive.indices < 0) {
        indices.reserve(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i) indices.push_back(static_cast<unsigned int>(i));
        return indices;
    }

    const auto& accessor = gltfModel.accessors[primitive.indices];
    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
    const auto& buffer = gltfModel.buffers[bufferView.buffer];

    indices.reserve(accessor.count);
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            const uint16_t* data = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i) indices.push_back(data[i]);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            const uint32_t* data = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            indices.assign(data, data + accessor.count);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i) indices.push_back(data[i]);
            break;
        }
    }

    return indices;
}

std::unique_ptr<Mesh> buildMeshFromPrimitive(const tinygltf::Model& gltfModel,
                                             const tinygltf::Primitive& primitive) {
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end()) return nullptr;

    const auto& posAccessor = gltfModel.accessors[posIt->second];
    if (posAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || posAccessor.type != TINYGLTF_TYPE_VEC3)
        return nullptr;

    const auto& posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
    const auto& posBuffer = gltfModel.buffers[posBufferView.buffer];
    const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
    const size_t vertexCount = posAccessor.count;

    std::vector<float> normals;
    if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
        const auto& nAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
        const auto& nBufferView = gltfModel.bufferViews[nAccessor.bufferView];
        const auto& nBuffer = gltfModel.buffers[nBufferView.buffer];
        const float* nData = reinterpret_cast<const float*>(&nBuffer.data[nBufferView.byteOffset + nAccessor.byteOffset]);
        normals.assign(nData, nData + nAccessor.count * 3);
    }

    std::vector<float> texCoords;
    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
        const auto& tAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
        const auto& tBufferView = gltfModel.bufferViews[tAccessor.bufferView];
        const auto& tBuffer = gltfModel.buffers[tBufferView.buffer];
        const float* tData = reinterpret_cast<const float*>(&tBuffer.data[tBufferView.byteOffset + tAccessor.byteOffset]);
        texCoords.assign(tData, tData + tAccessor.count * 2);
    }

    std::vector<float> vertices;
    vertices.reserve(vertexCount * 8);  // 3 pos + 3 normal + 2 tex

    AABB aabb;
    if (vertexCount > 0) aabb.min = aabb.max = glm::vec3(posData[0], posData[1], posData[2]);

    for (size_t i = 0; i < vertexCount; ++i) {
        glm::vec3 pos(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        if (i > 0) {
            aabb.min = glm::min(aabb.min, pos);
            aabb.max = glm::max(aabb.max, pos);
        }

        if (!normals.empty() && i * 3 + 2 < normals.size()) {
            vertices.push_back(normals[i * 3]);
            vertices.push_back(normals[i * 3 + 1]);
            vertices.push_back(normals[i * 3 + 2]);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        if (!texCoords.empty() && i * 2 + 1 < texCoords.size()) {
            vertices.push_back(texCoords[i * 2]);
            vertices.push_back(1.0f - texCoords[i * 2 + 1]);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    auto indices = readIndices(gltfModel, primitive, vertexCount);

    return std::make_unique<Mesh>(vertices.data(), vertices.size() * sizeof(float),
                                  indices.data(), indices.size(), aabb);
}

MaterialHandle resolveMaterial(const tinygltf::Primitive& primitive,
                               const std::vector<MaterialHandle>& materials,
                               const MaterialHandle& fallback) {
    if (primitive.material >= 0 && primitive.material < static_cast<int>(materials.size()))
        return materials[primitive.material];
    return fallback;
}

}

Model::Model(const std::string& gltfPath, const std::string& shaderPath, AssetManager& assetManager)
    : Asset(gltfPath) {
    tinygltf::Model gltfModel = loadGltfModel(gltfPath);
    std::string gltfDir = getDirectory(gltfPath);

    auto gltfTextures = loadGltfTextures(gltfModel, gltfDir, assetManager);
    auto shader = assetManager.getOrLoadShader(shaderPath);
    auto defaultMaterial = createDefaultMaterial(m_Path + "#default", assetManager, shader);
    auto gltfMaterials = buildMaterials(gltfModel, assetManager, shader, gltfTextures);

    size_t totalPrimitives = 0;
    for (const auto& mesh : gltfModel.meshes) totalPrimitives += mesh.primitives.size();
    m_SubMeshes.reserve(totalPrimitives);

    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            auto meshPtr = buildMeshFromPrimitive(gltfModel, primitive);
            if (!meshPtr) continue;
            auto mat = resolveMaterial(primitive, gltfMaterials, defaultMaterial);
            m_SubMeshes.push_back({std::move(meshPtr), mat});
        }
    }
}

std::string Model::getDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    return (lastSlash == std::string::npos) ? "." : filepath.substr(0, lastSlash);
}