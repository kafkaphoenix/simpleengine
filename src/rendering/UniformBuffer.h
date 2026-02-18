#pragma once

#include "GlBuffer.h"

class UniformBuffer {
   public:
    UniformBuffer(GLsizeiptr size, GLuint binding);
    ~UniformBuffer() = default;

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other) noexcept = default;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept = default;

    void update(GLsizeiptr size, const void* data) const;
    void updateSubData(GLintptr offset, GLsizeiptr size, const void* data) const;

   private:
    GlBuffer m_Buffer;
    GLuint m_Binding = 0;
};
