#version 330 core
in vec3 oNor;
in vec3 oCol;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(oCol, 1.0);
}
