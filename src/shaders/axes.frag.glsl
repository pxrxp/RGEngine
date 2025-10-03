#version 330 core
in vec3 oo_col;
out vec4 FragColor;

void main() {
    FragColor = vec4(oo_col, 1.0);
}
