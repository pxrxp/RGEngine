#version 330 core
in vec3 o_tex_coords;
uniform samplerCube u_skybox;

out vec4 FragColor;

void main() {
    FragColor = texture(u_skybox, o_tex_coords);
}

