#version 330 core
out vec4 FragColor;
layout(location = 1) out float out_color_mask;

void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    out_color_mask = 0.0;
}
