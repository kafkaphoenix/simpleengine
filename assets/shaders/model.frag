#version 450 core

layout (location = 0) in vec3 surface_normal;
layout (location = 1) in vec2 vtexture_coords;
layout (location = 2) in vec3 direction_to_light;
layout (location = 3) in vec3 direction_to_camera;
layout (location = 4) in float fog_visibility;
layout (location = 5) in vec4 world_position;
layout (location = 6) in vec4 world_normal;

layout (location = 0) out vec4 frag_color;

// TEXTURES
uniform sampler2D texture_diffuse_1;
uniform sampler2D texture_specular_1;
uniform sampler2D texture_normal_1;
uniform sampler2D texture_height_1;
// TODO rethink this two uniforms
uniform bool texture_enabled;
uniform bool normal_enabled;
// COLOR
uniform bool color_enabled;
uniform vec4 color;
// BLEND
uniform bool blend_texture_enabled;
uniform sampler2D blend_texture;
uniform float blend_texture_factor;
uniform bool blend_color_enabled;
uniform vec4 blend_color;
uniform float blend_color_factor;
// MATERIAL
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;
// FOG
uniform vec4 fog_color;
// LIGHTING
uniform bool light_enabled;
uniform vec3 light_color;
uniform float light_intensity;
uniform float light_range;
uniform float light_inner_cone_angle;
uniform float light_outer_cone_angle;
// REFLECTION
uniform vec3 camera_position;
uniform bool reflection_enabled;
uniform float reflectivity;
uniform float refractivity;
// SKYBOX
uniform bool blend_skybox_enabled;
uniform samplerCube skybox_texture;
uniform samplerCube blend_skybox_texture;
uniform float blend_skybox_factor;

void calculate_color() {
    if (texture_enabled) {
        if (normal_enabled) {
            frag_color = texture(texture_normal_1, vtexture_coords);
        } else {
            frag_color = texture(texture_diffuse_1, vtexture_coords);
            if (color_enabled) {
                if (blend_color_enabled) {
                    frag_color = mix(frag_color, blend_color, blend_color_factor);
                } else {
                    frag_color = color;
                }
            }
            if (blend_texture_enabled) {
                vec4 blend_texture_color = texture(blend_texture, vtexture_coords);
                frag_color = mix(frag_color, blend_texture_color, blend_texture_factor); 
            }
        }
    } else {
        frag_color = vec4(ambient, 1.f);
    }
}

void calculate_transparency() {
    if (frag_color.a < 0.1) {
        discard;
    }
}

void calculate_reflection() {
    if (reflection_enabled) {
        // REFLECTION
        vec3 I = normalize(world_position.xyz - camera_position);
        vec3 R = reflect(I, normalize(world_normal.xyz));
        vec4 reflection_color = texture(skybox_texture, R);
        if (blend_skybox_enabled) {
            vec4 blend_skybox_color = texture(blend_skybox_texture, R);
            reflection_color = mix(reflection_color, blend_skybox_color, blend_skybox_factor);
        }
        frag_color = mix(frag_color, reflection_color, reflectivity);
        // REFRACTION
        float ratio = 1.f / refractivity;
        vec3 Rr = refract(I, normalize(world_normal.xyz), refractivity);
        vec4 refraction_color = texture(skybox_texture, Rr);
        if (blend_skybox_enabled) {
            vec4 blend_skybox_color = texture(blend_skybox_texture, Rr);
            refraction_color = mix(refraction_color, blend_skybox_color, blend_skybox_factor);
        }
        frag_color = mix(frag_color, refraction_color, 0.5);
    }
}

void calculate_fog_visibility() {
    frag_color = mix(frag_color, fog_color, fog_visibility);
}

void calculate_lighting() {
    if (light_enabled) {
        vec3 ambient_component = ambient * light_color * light_intensity;
        // diffuse
        float diffuse_factor = max(dot(normalize(surface_normal), normalize(direction_to_light)), 0.f);
        vec3 diffuse_component = diffuse * light_color * light_intensity * diffuse_factor;
        // specular
        vec3 view_dir = normalize(camera_position - world_position.xyz);
        vec3 reflect_dir = reflect(-normalize(direction_to_light), normalize(surface_normal));
        float specular_factor = pow(max(dot(view_dir, reflect_dir), 0.f), shininess);
        vec3 specular_component = specular * light_color * light_intensity * specular_factor;
        frag_color += vec4(ambient_component + diffuse_component + specular_component, 1.0);
    }
}

void main() {
    calculate_color();
    calculate_transparency();
    calculate_reflection();
    calculate_lighting();
    calculate_fog_visibility();
}
