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

    // Checker pattern
    vec2 check_tile = floor(v_world_pos.xz);
    float checker = mod(check_tile.x + check_tile.y, 2.0);
    vec3 tile_col = mix(vec3(0.24, 0.26, 0.30), vec3(0.20, 0.22, 0.25), checker);
    vec3 grid_col = vec3(0.40, 0.44, 0.52);

    vec3 final_col = mix(tile_col, grid_col, c);

    // Fade into the horizon distance (fog) so it smoothly joins the skybox
    float dist = length(v_world_pos.xz);
    vec3 horizon_fog = vec3(0.70, 0.55, 0.45);
    float fog = clamp((dist - 30.0) / 60.0, 0.0, 1.0);
    final_col = mix(final_col, horizon_fog, fog);

    final_col = final_col / (final_col + vec3(1.0));
    final_col = pow(final_col, vec3(1.0 / 2.2));

    FragColor = vec4(final_col, 1.0);
}
