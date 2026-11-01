#version 330 core
in vec3 oo_col;
out vec4 FragColor;
layout(location = 1) out float out_color_mask;

void main() {
    FragColor = vec4(oo_col, 1.0);
    out_color_mask = 0.0;
}
