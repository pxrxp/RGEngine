#version 330 core
in vec3 o_tex_coords;
uniform samplerCube u_skybox;

out vec4 FragColor;
layout(location = 1) out float out_color_mask;

void main() {
    FragColor = texture(u_skybox, o_tex_coords);
    out_color_mask = 0.0;
}

