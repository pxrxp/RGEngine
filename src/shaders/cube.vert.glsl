#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_nor;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 v_world_pos;
out vec2 v_uv;
out vec3 v_normal;

void main() {
    vec4 world_pos = u_model * vec4(a_pos, 1.0);
    v_world_pos = world_pos.xyz;
    v_uv = a_uv;
    
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    v_normal = normalize(normal_matrix * a_nor);
    
    gl_Position = u_proj * u_view * world_pos;
}
