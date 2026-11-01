#version 330 core

layout (lines) in; 
layout (triangle_strip, max_vertices = 4) out; 
uniform float u_line_thickness; 

void main() {
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    vec2 dir = normalize(p2.xy - p1.xy);
    vec2 normal = vec2(-dir.y, dir.x);

    float half_width = u_line_thickness / 2.0;

    gl_Position = p1 + vec4(normal * half_width, 0.0, 0.0);
    EmitVertex();

    gl_Position = p1 - vec4(normal * half_width, 0.0, 0.0);
    EmitVertex();

    gl_Position = p2 + vec4(normal * half_width, 0.0, 0.0);
    EmitVertex();

    gl_Position = p2 - vec4(normal * half_width, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}
