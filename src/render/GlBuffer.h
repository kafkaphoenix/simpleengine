#pragma once
#include <glad/glad.h>

#include <span>

namespace se::render {

class GlBuffer {
   public:
    GlBuffer();
    ~GlBuffer();

    GlBuffer(const GlBuffer&) = delete;
    GlBuffer& operator=(const GlBuffer&) = delete;
    GlBuffer(GlBuffer&& other) noexcept;
    GlBuffer& operator=(GlBuffer&& other) noexcept;

    void setData(std::span<const std::byte> data, GLenum usage) const;
    void setData(GLsizeiptr size, GLenum usage) const;  // allocate only, no data
    void updateSubData(GLintptr offset, std::span<const std::byte> data) const;
    void* mapWrite(GLintptr offset, GLsizeiptr size) const;
    void unmap() const;
    unsigned int id() const { return m_Id; }

   private:
    void release();

    unsigned int m_Id = 0;
};

}  // namespace se::render
