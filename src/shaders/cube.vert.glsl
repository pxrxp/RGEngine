R"(#version 460 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragCol;

void main() {
    gl_Position = vec4(vertexPos, 1.0);
    fragCol = vertexCol;
})"
