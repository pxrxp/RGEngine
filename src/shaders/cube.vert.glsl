#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nor;
layout(location = 2) in vec3 a_col;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 o_nor;
out vec3 o_col;

void main() {
    o_nor = a_nor;
    o_col = a_col;
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
}
