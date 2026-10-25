#version 330 core

in vec3 v_world_pos;
in vec3 v_normal;
in vec3 v_col;

out vec4 FragColor;

uniform mat4 u_view;

void main() {
    vec3 N = normalize(v_normal);
    
    // Sunset sun direction and color
    vec3 L = normalize(vec3(0.6, 0.45, -0.7));
    vec3 sun_col = vec3(1.0, 0.82, 0.60);
    
    // Sky ambient and ground bounce
    vec3 sky_ambient = vec3(0.28, 0.32, 0.45);
    vec3 ground_ambient = vec3(0.15, 0.12, 0.12);
    float hemi = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambient = mix(ground_ambient, sky_ambient, hemi);

    // Diffuse with slight wrap
    float diff = max(dot(N, L), 0.0);
    float soft_diff = diff * 0.75 + 0.25;

    // Specular highlight (camera position extracted from inverse view matrix)
    vec3 cam_pos = inverse(u_view)[3].xyz;
    vec3 V = normalize(cam_pos - v_world_pos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * (diff > 0.0 ? 1.0 : 0.0);
    vec3 specular = sun_col * spec * 0.35;

    vec3 final_col = v_col * (ambient + sun_col * soft_diff) + specular;

    // Tone map & gamma
    final_col = final_col / (final_col + vec3(1.0));
    final_col = pow(final_col, vec3(1.0 / 2.2));

    FragColor = vec4(final_col, 1.0);
}
