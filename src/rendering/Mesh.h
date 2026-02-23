#pragma once
#include <glad/glad.h>

#include <cstddef>

#include "GlBuffer.h"
#include "VertexArray.h"

class Mesh {
   public:
    Mesh(float* vertices, unsigned int vertSize,
         unsigned int* indices, unsigned int idxCount);
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    void drawInstanced(unsigned int count) const;

    unsigned int getVAO() const { return m_Vao.id(); }
    unsigned int getIndexCount() const { return indexCount; }
    void updateInstanceBuffer(const void* data, size_t size) const;
    static void setDefaultInstanceCapacityBytes(size_t bytes);

   private:
    VertexArray m_Vao;
    GlBuffer m_Vbo{GL_ARRAY_BUFFER};
    GlBuffer m_Ebo{GL_ELEMENT_ARRAY_BUFFER};
    GlBuffer m_InstanceVbo{GL_ARRAY_BUFFER};
    mutable size_t m_InstanceCapacityBytes = 0;
    unsigned int indexCount = 0;
    static size_t s_DefaultInstanceCapacityBytes;
};
