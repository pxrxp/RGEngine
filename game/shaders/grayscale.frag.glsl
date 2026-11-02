#version 330 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_scene;
uniform sampler2D u_scene_aux;

const vec3 GRAYSCALE_WEIGHTS = vec3(0.299, 0.587, 0.114);

void main()
{
    vec4 color = texture(u_scene, v_uv);
    // RivalGrounds keeps the world desaturated and lets objects that matter
    // to the player opt out through the auxiliary scene attachment.
    float preserve_color = texture(u_scene_aux, v_uv).r;
    float gray = dot(color.rgb, GRAYSCALE_WEIGHTS) * 0.25;
    frag_color = vec4(mix(vec3(gray), color.rgb, preserve_color), color.a);
}
