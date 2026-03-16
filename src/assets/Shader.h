#pragma once
#include <glad/glad.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include "Asset.h"
#include "assets/StringHash.h"
#include "render/BufferLayout.h"

namespace se::assets {

class Shader : public Asset {
   public:
    explicit Shader(std::string shaderPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    void bind() const;
    void unbind() const;

    unsigned int id() const { return m_Id; }

    void validateLayout(const se::render::BufferLayout& layout,
                        GLuint instanceAttribBase) const;

    void setMat4(std::string_view name, const float* value);
    void setVec4(std::string_view name, const float* value);
    void setVec3(std::string_view name, const float* value);
    void setInt(std::string_view name, int value);
    void setFloat(std::string_view name, float value);
    void setBool(std::string_view name, bool value);

    std::string_view getPath() const override { return m_Path; }

   private:
    // Returns cached location, or queries and caches it on first call.
    int getUniformLocation(std::string_view name);

    unsigned int m_Id = 0;
    std::string m_Path;
    std::unordered_map<std::string, int, TransparentStringHash, std::equal_to<>> m_UniformLocations;
};

}  // namespace se::assets