#include "SkyboxRenderer.h"

namespace se::render {

SkyboxRenderer::SkyboxRenderer() : m_Shader("assets/shaders/skybox") { setupSampler(); }

SkyboxRenderer::~SkyboxRenderer() {
    if (m_Sampler) {
        glDeleteSamplers(1, &m_Sampler);
    }
}

void SkyboxRenderer::setCubemap(std::shared_ptr<se::assets::Cubemap> cubemap) { m_Cubemap = std::move(cubemap); }

void SkyboxRenderer::draw() {
    if (!m_Cubemap) {
        return;
    }

    // Depth trick: set depth to max (w component) in vertex shader, use LEQUAL depth test to draw skybox behind all
    // geometry with depth < 1.0
    glDepthFunc(GL_LEQUAL);

    m_Shader.bind();

    glBindTextureUnit(0, m_Cubemap->id());
    glBindSampler(0, m_Sampler);

    m_Vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindSampler(0, 0);
    glDepthFunc(GL_LESS);
}

void SkyboxRenderer::setupSampler() {
    glCreateSamplers(1, &m_Sampler);
    glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_Sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glObjectLabel(GL_SAMPLER, m_Sampler, -1, "SkyboxSampler");
}

}  // namespace se::render
