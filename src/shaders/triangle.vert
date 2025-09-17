#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat3 matr;
void main() {
    gl_Position = vec4(matr*aPos, 1.0);
}
