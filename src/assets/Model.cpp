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

    bool ret = false;
    if (gltfPath.find(".glb") != std::string::npos) {
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, gltfPath);
    } else {
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfPath);
    }

    if (!ret) {
        throw std::runtime_error("Failed to load GLTF: " + err);
    }

    if (!warn.empty()) {
        std::cout << "GLTF Warning: " << warn << std::endl;
    }

    return gltfModel;
}

std::vector<TextureHandle> loadGltfTextures(const tinygltf::Model& gltfModel,
                                            const std::string& gltfDir,
                                            AssetManager& assetManager) {
    std::vector<TextureHandle> gltfTextures;
    gltfTextures.reserve(gltfModel.textures.size());

    for (size_t i = 0; i < gltfModel.textures.size(); ++i) {
        const auto& texture = gltfModel.textures[i];
        if (texture.source >= 0 && texture.source < gltfModel.images.size()) {
            const auto& image = gltfModel.images[texture.source];

            if (!image.uri.empty()) {
                std::string texturePath = gltfDir + "/" + image.uri;
                try {
                    auto existingHandle = assetManager.getTextureHandle(texturePath);
                    if (existingHandle.isValid()) {
                        gltfTextures.push_back(existingHandle);
                    } else {
                        auto textureHandle = assetManager.loadTexture(texturePath);
                        gltfTextures.push_back(textureHandle);
                    }
                } catch (const std::exception& e) {
                    throw std::runtime_error("Failed to load GLTF texture " + std::to_string(i) +
                                             " (" + image.uri + "): " + e.what());
                }
            } else {
                throw std::runtime_error("Embedded texture " + std::to_string(i) + " not supported");
            }
        } else {
            throw std::runtime_error("Invalid texture source for texture " + std::to_string(i));
        }
    }

    return gltfTextures;
}

MaterialHandle createDefaultMaterial(const std::string& name,
                                     AssetManager& assetManager,
                                     const ShaderHandle& shader) {
    MaterialTextures defaultTextures;
    MaterialParams defaultParams;
    return assetManager.createMaterial(name, shader, defaultTextures, defaultParams, RenderState{});
}

std::vector<MaterialHandle> buildMaterials(const tinygltf::Model& gltfModel,
                                           AssetManager& assetManager,
                                           const ShaderHandle& shader,
                                           const std::vector<TextureHandle>& textures,
                                           const std::string& modelPath) {
    std::vector<MaterialHandle> gltfMaterials;
    gltfMaterials.reserve(gltfModel.materials.size());

    for (size_t i = 0; i < gltfModel.materials.size(); ++i) {
        const auto& material = gltfModel.materials[i];
        MaterialTextures matTextures;
        MaterialParams params;

        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            int texIndex = material.pbrMetallicRoughness.baseColorTexture.index;
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size())) {
                matTextures.baseColor = textures[texIndex];
            }
        }

        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
            int texIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size())) {
                matTextures.metallicRoughness = textures[texIndex];
            }
        }

        if (material.normalTexture.index >= 0) {
            int texIndex = material.normalTexture.index;
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size())) {
                matTextures.normal = textures[texIndex];
            }
        }

        if (material.emissiveTexture.index >= 0) {
            int texIndex = material.emissiveTexture.index;
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size())) {
                matTextures.emissive = textures[texIndex];
            }
        }

        if (material.occlusionTexture.index >= 0) {
            int texIndex = material.occlusionTexture.index;
            if (texIndex >= 0 && texIndex < static_cast<int>(textures.size())) {
                matTextures.occlusion = textures[texIndex];
            }
        }

        if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
            params.baseColorFactor = glm::vec4(
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3]));
        }

        params.metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
        params.roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);

        if (material.emissiveFactor.size() == 3) {
            params.emissiveFactor = glm::vec3(
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2]));
        }

        if (material.alphaMode == "MASK") {
            params.alphaCutoff = static_cast<float>(material.alphaCutoff);
        } else {
            params.alphaCutoff = 0.0f;
        }

        RenderState state;
        if (material.alphaMode == "BLEND") {
            state.blend = true;
            state.depthWrite = false;
        } else {
            state.blend = false;
            state.depthWrite = true;
        }

        std::string name = modelPath + "#mat" + std::to_string(i);
        gltfMaterials.push_back(assetManager.createMaterial(name, shader, matTextures, params, state));
    }

    return gltfMaterials;
}

std::vector<float> readVec3Attribute(const tinygltf::Model& gltfModel,
                                     const tinygltf::Primitive& primitive,
                                     const char* attributeName) {
    std::vector<float> values;
    auto attrIt = primitive.attributes.find(attributeName);
    if (attrIt == primitive.attributes.end()) {
        return values;
    }

    const auto& accessor = gltfModel.accessors[attrIt->second];
    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC3) {
        return values;
    }

    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
    const auto& buffer = gltfModel.buffers[bufferView.buffer];
    const float* data = reinterpret_cast<const float*>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    values.assign(data, data + accessor.count * 3);
    return values;
}

std::vector<float> readVec2Attribute(const tinygltf::Model& gltfModel,
                                     const tinygltf::Primitive& primitive,
                                     const char* attributeName) {
    std::vector<float> values;
    auto attrIt = primitive.attributes.find(attributeName);
    if (attrIt == primitive.attributes.end()) {
        return values;
    }

    const auto& accessor = gltfModel.accessors[attrIt->second];
    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC2) {
        return values;
    }

    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
    const auto& buffer = gltfModel.buffers[bufferView.buffer];
    const float* data = reinterpret_cast<const float*>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    values.assign(data, data + accessor.count * 2);
    return values;
}

std::vector<unsigned int> readIndices(const tinygltf::Model& gltfModel,
                                      const tinygltf::Primitive& primitive,
                                      size_t vertexCount) {
    std::vector<unsigned int> indices;
    if (primitive.indices < 0) {
        indices.reserve(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i) {
            indices.push_back(static_cast<unsigned int>(i));
        }
        return indices;
    }

    const auto& accessor = gltfModel.accessors[primitive.indices];
    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
    const auto& buffer = gltfModel.buffers[bufferView.buffer];

    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const uint16_t* data = reinterpret_cast<const uint16_t*>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        indices.reserve(accessor.count);
        for (size_t i = 0; i < accessor.count; ++i) {
            indices.push_back(static_cast<unsigned int>(data[i]));
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        const uint32_t* data = reinterpret_cast<const uint32_t*>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        indices.assign(data, data + accessor.count);
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        indices.reserve(accessor.count);
        for (size_t i = 0; i < accessor.count; ++i) {
            indices.push_back(static_cast<unsigned int>(data[i]));
        }
    }

    return indices;
}

std::vector<float> buildVertices(const float* posData,
                                 size_t vertexCount,
                                 const std::vector<float>& normals,
                                 const std::vector<float>& texCoords) {
    std::vector<float> vertices;
    vertices.reserve(vertexCount * 8);
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices.push_back(posData[i * 3]);
        vertices.push_back(posData[i * 3 + 1]);
        vertices.push_back(posData[i * 3 + 2]);

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

    return vertices;
}

bool checkAccessorMinMax(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive) {
    auto& accessor = gltfModel.accessors[primitive.indices];
    return !accessor.minValues.empty() && !accessor.maxValues.empty() &&
           accessor.minValues.size() == accessor.maxValues.size();
}

AABB computeAABB(const float* posData, const tinygltf::Accessor& accessor, bool hasMinMax) {
    AABB aabb;
    if (hasMinMax) {
        aabb.min = glm::vec3(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
        aabb.max = glm::vec3(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);
        return aabb;
    }

    aabb.min = glm::vec3(posData[0], posData[1], posData[2]);
    aabb.max = aabb.min;

    for (size_t i = 1; i < accessor.count; ++i) {
        glm::vec3 pos(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
        aabb.min = glm::min(aabb.min, pos);
        aabb.max = glm::max(aabb.max, pos);
    }

    return aabb;
}

std::unique_ptr<Mesh> buildMeshFromPrimitive(const tinygltf::Model& gltfModel,
                                             const tinygltf::Primitive& primitive) {
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end()) {
        return nullptr;
    }

    const auto& posAccessor = gltfModel.accessors[posIt->second];
    if (posAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || posAccessor.type != TINYGLTF_TYPE_VEC3) {
        return nullptr;
    }
    const auto& posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
    const auto& posBuffer = gltfModel.buffers[posBufferView.buffer];
    const float* posData = reinterpret_cast<const float*>(
        &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
    bool hasAABB = checkAccessorMinMax(gltfModel, primitive);

    auto normals = readVec3Attribute(gltfModel, primitive, "NORMAL");
    auto texCoords = readVec2Attribute(gltfModel, primitive, "TEXCOORD_0");
    auto indices = readIndices(gltfModel, primitive, posAccessor.count);
    auto vertices = buildVertices(posData, posAccessor.count, normals, texCoords);
    auto aabb = computeAABB(posData, posAccessor, hasAABB);
    return std::make_unique<Mesh>(vertices.data(), vertices.size() * sizeof(float),
                                  indices.data(), indices.size(), aabb);
}

MaterialHandle resolveMaterial(const tinygltf::Primitive& primitive,
                               const std::vector<MaterialHandle>& materials,
                               const MaterialHandle& fallback) {
    if (primitive.material >= 0 && primitive.material < static_cast<int>(materials.size())) {
        return materials[primitive.material];
    }
    return fallback;
}
}

Model::Model(const std::string& gltfPath, const std::string& shaderPath, AssetManager& assetManager)
    : Asset(gltfPath) {
    tinygltf::Model gltfModel = loadGltfModel(gltfPath);
    std::string gltfDir = getDirectory(gltfPath);
    auto gltfTextures = loadGltfTextures(gltfModel, gltfDir, assetManager);

    ShaderHandle shader = assetManager.loadShader(shaderPath);
    MaterialHandle defaultMaterial = createDefaultMaterial(m_Path + "#default", assetManager, shader);
    auto gltfMaterials = buildMaterials(gltfModel, assetManager, shader, gltfTextures, m_Path);

    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            auto meshPtr = buildMeshFromPrimitive(gltfModel, primitive);
            if (!meshPtr) {
                continue;
            }

            MaterialHandle materialHandle = resolveMaterial(primitive, gltfMaterials, defaultMaterial);
            m_SubMeshes.push_back({std::move(meshPtr), materialHandle});
        }
    }
}

std::string Model::getDirectory(const std::string& filepath) const {
    size_t found = filepath.find_last_of("/\\");
    if (found != std::string::npos) {
        return filepath.substr(0, found);
    }
    return ".";
}
