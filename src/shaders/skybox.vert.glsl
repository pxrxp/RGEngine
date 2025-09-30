#version 330 core
layout(location=0) in vec3 a_pos;

uniform mat4 u_proj;
uniform mat4 u_view;

out vec3 o_tex_coords;

void main() {
    o_tex_coords = a_pos;

    vec4 pos = u_proj * mat4(mat3(u_view)) * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
}
