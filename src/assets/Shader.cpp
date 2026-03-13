#include "Shader.h"

#include <glad/glad.h>

#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace se::assets {

static std::string loadFile(std::string_view path) {
    // open file in binary mode and move the file pointer to the end of the file to get the file size
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open shader file: {}", path));
    }

    // get file size
    const std::streamsize size = file.tellg();
    // reset file pointer to the beginning of the file
    file.seekg(0);

    // read file contents into a string with exactly the size of the file
    std::string buffer(size, '\0');
    file.read(buffer.data(), size);
    return buffer;
}

static void checkShaderCompilation(unsigned int shader, std::string_view type) {
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        throw std::runtime_error(std::format("{} shader compilation failed: {}", type, infoLog));
    }
}

static void checkProgramLinking(unsigned int program) {
    int success;
    char infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        throw std::runtime_error(std::format("Shader program linking failed: {}", infoLog));
    }
}

Shader::Shader(std::string shaderPath)
    : Asset(shaderPath), m_Path(std::move(shaderPath)) {
    std::string vSrc = loadFile(std::format("{}.vert", m_Path));
    std::string fSrc = loadFile(std::format("{}.frag", m_Path));

    const char* v = vSrc.c_str();
    const char* f = fSrc.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &v, nullptr);
    glCompileShader(vs);
    checkShaderCompilation(vs, "VERTEX");

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &f, nullptr);
    glCompileShader(fs);
    checkShaderCompilation(fs, "FRAGMENT");

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vs);
    glAttachShader(m_ID, fs);
    glLinkProgram(m_ID);
    checkProgramLinking(m_ID);

#ifdef GL_KHR_debug
    if (glad_glObjectLabel != nullptr) {
        glad_glObjectLabel(GL_PROGRAM, m_ID, static_cast<GLsizei>(m_Path.size()), m_Path.c_str());
    }
#endif

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Bind FrameData uniform block to binding 0 after linking
    unsigned int index = glGetUniformBlockIndex(m_ID, "FrameData");
    if (index != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_ID, index, 0);
    }
}

Shader::~Shader() { glDeleteProgram(m_ID); }

void Shader::bind() const { glUseProgram(m_ID); }
void Shader::unbind() const { glUseProgram(0); }

// Returns the location of the uniform variable with the given name, or -1 if it doesn't exist.
int Shader::getUniformLocation(std::string_view name) {
    auto it = m_UniformLocations.find(name);
    if (it != m_UniformLocations.end()) {
        return it->second;
    }

    // we save the uniform location in the map even if it's -1 (not found) to avoid redundant
    // glGetUniformLocation calls for the same name in the future.
    std::string nameStr(name);
    int loc = glGetUniformLocation(m_ID, nameStr.c_str());
    m_UniformLocations.emplace(nameStr, loc);
    return loc;
}

void Shader::setMat4(std::string_view name, const float* value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, value);
    }
}

void Shader::setVec4(std::string_view name, const float* value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4fv(loc, 1, value);
    }
}

void Shader::setVec3(std::string_view name, const float* value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3fv(loc, 1, value);
    }
}

void Shader::setInt(std::string_view name, int value) {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::setFloat(std::string_view name, float value) {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1f(loc, value);
}

void Shader::setBool(std::string_view name, bool value) {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value ? 1 : 0);
}

}  // namespace se::assets
