#version 330 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_scene;
uniform sampler2D u_scene_mask;

const vec3 GRAYSCALE_WEIGHTS = vec3(0.299, 0.587, 0.114);

void main()
{
    vec4 color = texture(u_scene, v_uv);
    float preserve_color = texture(u_scene_mask, v_uv).r;
    float gray = dot(color.rgb, GRAYSCALE_WEIGHTS);
    gray *= 0.25;
    vec3 output_color = mix(vec3(gray), color.rgb, preserve_color);
    frag_color = vec4(output_color, color.a);
}
