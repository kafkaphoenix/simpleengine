#include "Material.h"

Material::Material(const std::string& name,
                   ShaderHandle shader,
                   const MaterialTextures& textures,
                   const MaterialParams& params,
                   const RenderState& state)
    : Asset(name), m_Shader(shader), m_Textures(textures), m_Params(params), m_State(state) {}
