#pragma once
#include <glad/glad.h>

#include "VertexArray.h"
#include "assets/AssetHandle.h"
#include "assets/Shader.h"

namespace se::render {

// Renders a skybox as the last geometry pass step using the depth trick (z = w).
// Drawn after all scene objects so early-z discards most fragments.
// Reads view/projection from the shared FrameData UBO (binding 0), strips translation in-shader.
class SkyboxRenderer {
public:
    SkyboxRenderer();
    ~SkyboxRenderer();

    SkyboxRenderer(const SkyboxRenderer&) = delete;
    SkyboxRenderer& operator=(const SkyboxRenderer&) = delete;
    SkyboxRenderer(SkyboxRenderer&&) = delete;
    SkyboxRenderer& operator=(SkyboxRenderer&&) = delete;

    void setCubemap(se::assets::CubemapHandle cubemap);
    void draw();

    [[nodiscard]] bool hasCubemap() const { return m_Cubemap.isValid(); }

private:
    void setupSampler();

    se::assets::Shader m_Shader;
    VertexArray m_Vao;  // empty VAO for attributeless cube draw
    GLuint m_Sampler = 0;
    se::assets::CubemapHandle m_Cubemap;
};

}  // namespace se::render
