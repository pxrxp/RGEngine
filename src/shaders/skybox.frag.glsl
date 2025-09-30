#version 330 core
in vec3 texCoords;
uniform samplerCube u_skybox;

out vec4 FragColor;

void main() {
    FragColor = texture(u_skybox, texCoords);
}

