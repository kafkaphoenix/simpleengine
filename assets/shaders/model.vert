#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

layout (location = 0) out vec3 surface_normal;
layout (location = 1) out vec2 vtexture_coords;
layout (location = 2) out vec3 direction_to_light;
layout (location = 3) out vec3 direction_to_camera;
layout (location = 4) out float fog_visibility;
layout (location = 5) out vec4 world_position;
layout (location = 6) out vec4 world_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
// LIGHTING
uniform vec3 light_position;
// FOG
uniform bool fog_enabled;
uniform float fog_density;
uniform float fog_gradient;

void calculate_reflection() {
    surface_normal = (model * vec4(normal, 0.f)).xyz;
    direction_to_light = light_position - world_position.xyz;
    direction_to_camera = (inverse(view) * vec4(0.f, 0.f, 0.f, 1.f)).xyz - world_position.xyz;
}

void calculate_fog_visibility(vec4 view_position) {
    if (fog_enabled) {
        float distance_relative_to_camera = length(view_position.xyz);
        fog_visibility = exp(-pow((distance_relative_to_camera * fog_density), fog_gradient));
        fog_visibility = clamp(fog_visibility, 0.f, 1.f);
    } else {
        fog_visibility = 0.f;
    }
}

void main() {
    vec4 world_position = model * vec4(position, 1.f);
    vec3 world_normal = mat3(transpose(inverse(model))) * normal;
    vec4 view_position = view * world_position;
    vec4 clip_position = projection * view_position;
    gl_Position = clip_position;
    vtexture_coords = texture_coords;
    calculate_reflection();
    calculate_fog_visibility(view_position);
}