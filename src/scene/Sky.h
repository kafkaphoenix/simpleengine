#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <optional>
#include <string>

namespace se::scene {

class Sky {
public:
    void setAmbientColor(glm::vec3 color) { m_AmbientColor = color; }
    void setAmbientIntensity(float strength) { m_AmbientIntensity = strength; }
    void setCubemapFaces(std::array<std::string, 6> faces) { m_CubemapFaces = std::move(faces); }

    [[nodiscard]] glm::vec3 getAmbientColor() const { return m_AmbientColor; }
    [[nodiscard]] float getAmbientIntensity() const { return m_AmbientIntensity; }
    [[nodiscard]] const std::optional<std::array<std::string, 6>>& getCubemapFaces() const { return m_CubemapFaces; }

private:
    glm::vec3 m_AmbientColor{1.0f, 1.0f, 1.0f};
    float m_AmbientIntensity = 0.4f;
    std::optional<std::array<std::string, 6>> m_CubemapFaces;
};

}  // namespace se::scene