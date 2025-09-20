#version 330 core
layout(location=0) in vec3 aPos;

uniform mat4 proj;
uniform mat4 view;

out vec3 texCoords;

void main() {
    texCoords = aPos;

    vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
