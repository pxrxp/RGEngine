#version 330 core

in vec3 v_world_pos;
out vec4 FragColor;

void main() {
    // World-space seamless texture tiling (tiles every ~4 meters)
    vec2 uv = v_world_pos.xz * 0.25;
    vec3 ground_col = vec3(0.32, 0.30, 0.27);

    if (u_has_texture == 1) {
        ground_col = texture(u_ground_tex, uv).rgb;
    }

    // Directional sunlight & twilight ambient matching the sunset sky
    vec3 L = normalize(vec3(0.65, 0.40, -0.65));
    vec3 sun_col = vec3(1.0, 0.85, 0.65);
    vec3 sky_ambient = vec3(0.35, 0.38, 0.48);
    float diff = max(dot(vec3(0, 1, 0), L), 0.0);

    vec3 final_col = ground_col * (sky_ambient + sun_col * (diff * 0.7 + 0.3));

    // Atmospheric horizon haze: fades terrain into sunset skybox between 150m and 750m
    float dist = length(v_world_pos.xz);
    vec3 horizon_fog = vec3(0.72, 0.54, 0.44);
    float fog = clamp((dist - 150.0) / 600.0, 0.0, 1.0);
    final_col = mix(final_col, horizon_fog, fog);

    // Tone mapping & gamma
    final_col = final_col / (final_col + vec3(1.0));
    final_col = pow(final_col, vec3(1.0 / 2.2));

    FragColor = vec4(final_col, 1.0);
}
