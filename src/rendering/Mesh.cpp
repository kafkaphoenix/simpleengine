#include "Mesh.h"

#include <glm/glm.hpp>
#include <stdexcept>

#include "GlUtils.h"

Mesh::Mesh(float* vertices, unsigned int vertSize,
           unsigned int* indices, unsigned int idxCount)
    : indexCount(idxCount) {
    if (!vertices || !indices || vertSize == 0 || idxCount == 0) {
        throw std::invalid_argument("Invalid mesh data provided!");
    }

    m_Vbo.setData(vertSize, vertices, GL_STATIC_DRAW);

    m_Ebo.setData(
        idxCount * sizeof(unsigned int),
        indices, GL_STATIC_DRAW);

    m_Vao.setVertexBuffer(0, m_Vbo.id(), 0, 8 * sizeof(float));
    m_Vao.setElementBuffer(m_Ebo.id());

    // Position attribute (location = 0)
    m_Vao.enableAttrib(0);
    m_Vao.setAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    m_Vao.setAttribBinding(0, 0);

    // Normal attribute (location = 1)
    m_Vao.enableAttrib(1);
    m_Vao.setAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    m_Vao.setAttribBinding(1, 0);

    // Texture coordinate attribute (location = 2)
    m_Vao.enableAttrib(2);
    m_Vao.setAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    m_Vao.setAttribBinding(2, 0);
    // Reserve space for instance data
    m_InstanceVbo.setData(sizeof(glm::mat4) * 1000, nullptr, GL_DYNAMIC_DRAW);

    m_Vao.setVertexBuffer(1, m_InstanceVbo.id(), 0, sizeof(glm::mat4));

    // Setup instance matrix attributes (locations 3-6)
    for (int i = 0; i < 4; i++) {
        m_Vao.enableAttrib(3 + i);
        m_Vao.setAttribFormat(
            3 + i, 4, GL_FLOAT, GL_FALSE,
            static_cast<GLuint>(sizeof(glm::vec4) * i));
        m_Vao.setAttribBinding(3 + i, 1);
    }
    m_Vao.setBindingDivisor(1, 1);

    checkGlError("Mesh::Mesh");
}

void Mesh::drawInstanced(unsigned int count) const {
    if (count == 0) return;

    m_Vao.bind();

    glDrawElementsInstanced(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        nullptr,
        count);

    checkGlError("Mesh::drawInstanced");

    VertexArray::unbind();
}

void Mesh::updateInstanceBuffer(const void* data, size_t size) const {
    m_InstanceVbo.setData(static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_DRAW);
}
