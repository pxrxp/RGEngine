#version 330 core

in vec3 v_world_pos;
in vec2 v_uv;
in vec3 v_normal;

out vec4 FragColor;

uniform mat4 u_view;
uniform vec3 u_color = vec3(0.0);
uniform sampler2D u_diffuse;
uniform int u_use_texture = 0;
uniform int u_is_gun = 0;
uniform int u_preserve_color = 0;

void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(vec3(0.65, 0.55, -0.50));
    vec3 sun_col = vec3(1.0, 0.95, 0.85);
    
    vec3 sky_ambient = vec3(0.40, 0.42, 0.48);
    vec3 ground_ambient = vec3(0.22, 0.20, 0.20);
    float hemi = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambient = mix(ground_ambient, sky_ambient, hemi);

    float diff = max(dot(N, L), 0.0);
    float soft_diff = diff * 0.70 + 0.30;

    vec3 cam_pos = inverse(u_view)[3].xyz;
    vec3 V = normalize(cam_pos - v_world_pos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * (diff > 0.0 ? 1.0 : 0.0);

    vec3 base_color = u_color;
    if (u_use_texture == 1) {
        if (u_is_gun == 1) {
            base_color = texture(u_diffuse, v_uv).rgb * vec3(0.25, 0.26, 0.28);
        } else {
            base_color = texture(u_diffuse, v_uv).rgb;
        }
    } else if (length(base_color) < 0.001) {
        base_color = vec3(0.55, 0.53, 0.52);
    }

    vec3 final_col = base_color * (ambient + sun_col * soft_diff) + specular;

    // Subtle atmospheric distance haze
    float dist = length(v_world_pos.xz);
    vec3 haze_color = vec3(0.70, 0.55, 0.45);
    float haze = clamp((dist - 35.0) / 70.0, 0.0, 0.7);
    final_col = mix(final_col, haze_color, haze);

    // Filmic tone map & gamma correction
    final_col = final_col / (final_col + vec3(1.0));
    final_col = pow(final_col, vec3(1.0 / 2.2));

    FragColor = vec4(final_col, 1.0);
}
