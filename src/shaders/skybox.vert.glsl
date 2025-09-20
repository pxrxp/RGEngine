#version 330 core
layout(location=0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 texCoords;

void main() {
    texCoords = aPos;

    vec4 pos = projection * view * aPos;
    gl_Position = pos.xyww;
}
