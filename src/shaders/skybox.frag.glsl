#version 330 core
in vec3 texCoords;
// uniform samplerCube skybox;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(1.0);
    // FragColor = texture(skybox, texCoords);
}

