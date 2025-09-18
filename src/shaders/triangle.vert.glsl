#version 330 core
layout(location = 0) in vec3 aPos;
void main() {
    vec3 pos = aPos;
    float z = 0.01 + (1.0-0.01)* (pos.z+1.0)/2.0;
    pos.xy = pos.xy / z;
    gl_Position = vec4(pos, 1.0);
}
