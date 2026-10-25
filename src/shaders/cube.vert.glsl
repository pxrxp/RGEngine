#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_col;
layout(location = 2) in vec3 a_nor;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 v_world_pos;
out vec3 v_normal;
out vec3 v_col;

void main() {
    vec4 world_pos = u_model * vec4(a_pos, 1.0);
    v_world_pos = world_pos.xyz;
    
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    v_normal = normalize(normal_matrix * a_nor);
    
    v_col = a_col;
    gl_Position = u_proj * u_view * world_pos;
}
