#version 330 core

in vec3 v_world_pos;
in vec3 v_normal;
in vec3 v_col;

out vec4 FragColor;

uniform mat4 u_view;
uniform vec3 u_color = vec3(0.0);

void main() {
    vec3 N = normalize(v_normal);
    
    // Golden hour sunlight
    vec3 L = normalize(vec3(0.65, 0.40, -0.65));
    vec3 sun_col = vec3(1.0, 0.85, 0.65);
    
    // Ambient hemisphere light (warm ground, cool twilight sky)
    vec3 sky_ambient = vec3(0.30, 0.32, 0.45);
    vec3 ground_ambient = vec3(0.18, 0.14, 0.12);
    float hemi = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambient = mix(ground_ambient, sky_ambient, hemi);

    // Diffuse light with soft wrap
    float diff = max(dot(N, L), 0.0);
    float soft_diff = diff * 0.75 + 0.25;

    // Specular highlight
    vec3 cam_pos = inverse(u_view)[3].xyz;
    vec3 V = normalize(cam_pos - v_world_pos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * (diff > 0.0 ? 1.0 : 0.0);
    vec3 specular = sun_col * spec * 0.25;

    // Pick uniform color if supplied, else vertex color
    vec3 base_color = (length(u_color) > 0.001) ? u_color : v_col;
    if (length(base_color) < 0.001) {
        base_color = vec3(0.75, 0.70, 0.65);
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
