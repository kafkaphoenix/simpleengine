#include "Cubemap.h"

#include <stb_image.h>

#include <format>
#include <stdexcept>

namespace se::assets {

Cubemap::Cubemap(const std::array<std::string, 6>& facePaths) : Asset(facePaths[0]) {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Id);

    // Load first face to determine dimensions (all faces must match)
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(0);  // cubemap faces are NOT flipped
    unsigned char* probe = stbi_load(facePaths[0].c_str(), &width, &height, &channels, 0);
    if (!probe) {
        throw std::runtime_error(std::format("Failed to load cubemap face: {}", facePaths[0]));
    }
    stbi_image_free(probe);

    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    // Allocate immutable storage for all 6 faces at once
    glTextureStorage2D(m_Id, 1, internalFormat, width, height);

    // Upload each face
    for (unsigned int i = 0; i < 6; ++i) {
        int w = 0;
        int h = 0;
        int ch = 0;
        unsigned char* data = stbi_load(facePaths[i].c_str(), &w, &h, &ch, channels);
        if (!data) {
            throw std::runtime_error(std::format("Failed to load cubemap face: {}", facePaths[i]));
        }
        if (w != width || h != height) {
            stbi_image_free(data);
            throw std::runtime_error(std::format("Cubemap face size mismatch: {} is {}x{}, expected {}x{}",
                                                 facePaths[i], w, h, width, height));
        }

        // For cubemaps, zoffset = face index (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
        glTextureSubImage3D(m_Id, 0, 0, 0, static_cast<GLint>(i), width, height, 1, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    std::string label = std::format("Cubemap [{}]", m_Name);
    glObjectLabel(GL_TEXTURE, m_Id, static_cast<GLsizei>(label.size()), label.c_str());
}

Cubemap::~Cubemap() {
    if (m_Id != 0) {
        glDeleteTextures(1, &m_Id);
    }
}

}  // namespace se::assets
