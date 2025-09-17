#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat3 mat;
void main() {
    gl_Position = vec4(mat*aPos, 1.0);
}
