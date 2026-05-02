#version 460 core

out vec3 v_TexCoord;

layout(std140, binding = 0) uniform FrameData {
    mat4 u_View;
    mat4 u_Projection;
};

const vec3 CUBE_POSITIONS[36] = vec3[](
    // +X face
    vec3(1, -1, -1), vec3(1, -1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, -1), vec3(1, -1, -1),
    // -X face
    vec3(-1, -1, 1), vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(-1, 1, -1), vec3(-1, 1, 1), vec3(-1, -1, 1),
    // +Y face
    vec3(-1, 1, -1), vec3(1, 1, -1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(-1, 1, 1), vec3(-1, 1, -1),
    // -Y face
    vec3(-1, -1, 1), vec3(1, -1, 1), vec3(1, -1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, -1, 1),
    // +Z face
    vec3(-1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1),
    // -Z face
    vec3(1, -1, -1), vec3(1, 1, -1), vec3(-1, 1, -1), vec3(-1, 1, -1), vec3(-1, -1, -1), vec3(1, -1, -1));

void main() {
    vec3 pos = CUBE_POSITIONS[gl_VertexID];
    v_TexCoord = pos;

    // Strip translation from view so skybox stays centered on camera.
    // Depth trick: set z = w so after perspective divide depth = 1.0 (far plane).
    vec4 clipPos = u_Projection * mat4(mat3(u_View)) * vec4(pos, 1.0);
    gl_Position = clipPos.xyww;
}
