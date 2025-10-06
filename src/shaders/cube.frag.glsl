#version 330 core
in vec3 o_nor;
in vec3 o_col;

out vec4 FragColor;

void main() {
    FragColor = vec4(o_nor, 1.0);
}
