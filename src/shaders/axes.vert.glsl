#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 o_col;

void main() {
    o_col = a_pos;
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
}
